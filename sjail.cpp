#include <iostream>
#include <cstdlib>
#include <unordered_map>

#include <unistd.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>

#include "sjail.h"
#include "report.h"
#include "config.h"
#include "signal_tab.h"
#include "filter.h"
#include "memory.h"
#include "process_state.h"

using namespace std;

static std::unordered_map<pid_t, pid_data> proc_data;

int syscall_failed(const char* msg) {
  perror(msg);
  return 1;
}

void setlimit(int res, int softlimit) {
  struct rlimit rl;
  rl.rlim_cur = softlimit;
  rl.rlim_max = softlimit+softlimit;
  if(setrlimit(res, &rl)) {
    exit(syscall_failed("setrlimit"));
  }
}

int teardown_processes(const char* msg) {
  if(msg) {
    fprintf(stderr, "%s\n", msg);
  }
  for(auto i : proc_data) {
    syscall(SYS_tkill, i.first, SIGKILL);
  }
  proc_data.clear();
  return 1;
}

bool cleanup_process(pid_t pid, exit_data& data) {
  pid_data* pdata = &proc_data[pid];
  for(auto i : pdata->filters) {
    i->on_exit(*pdata, data);
    if(i->unref()) {
      delete i;
    }
  }
  proc_data.erase(pid);

  log_exit(pid, data, proc_data.empty());
  if(proc_data.empty()) {
    finalize_report();
    return true;
  }
  return false;
}

static volatile bool alarm_fired = false;
static volatile bool in_wait = false;

static void handle_sigalrm(int signal) {
  if(in_wait) {
    teardown_processes(NULL);
  } else {
    alarm_fired = true;
  }
}

int sjail_child(char** argv) {
  if(!safemem_map_unwritable()) {
    return syscall_failed("failed to map memory");
  }

  /* Setup chroot if specified. */
#ifdef PATH_MAX
  char chroot_path[PATH_MAX];
#else
  char chroot_path[4096];
#endif
  if(!get_chroot().empty()) {
    if(chroot_path != realpath(get_chroot().c_str(), chroot_path)) {
      return syscall_failed("Failed to find real path of chroot");
    }
  }
  if(!get_cwd().empty()) {
    if(chdir(get_cwd().c_str())) {
      return syscall_failed("Failed to change working directory");
    }
  }
  if(!get_chroot().empty()) {
    if(chroot(chroot_path)) {
      return syscall_failed("Failed to change root directory");
    }
  }

  /* Set resource limits. */
  if(get_time() != TIME_NO_LIMIT) {
    setlimit(RLIMIT_CPU, get_time());
  }
  if(get_mem() != MEM_NO_LIMIT) {
    setlimit(RLIMIT_AS, get_mem());
  }
  if(get_file_limit() != FILE_NO_LIMIT){
    setlimit(RLIMIT_FSIZE, get_file_limit());
  }

  /* Set user and groups. */
  struct passwd* pw_user = NULL;
  struct passwd* pw_group = NULL;
  if(!get_group().empty()) {
    pw_group = getpwnam(get_group().c_str());
    if(!pw_group) {
      return syscall_failed("Failed to look up group id");
    }
  }
  if(!get_user().empty()) {
    pw_user = getpwnam(get_user().c_str());
    if(!pw_user) {
      return syscall_failed("Failed to look up user id");
    }
  }

  if(pw_group) {
    if(setresgid(pw_group->pw_uid, pw_group->pw_uid, pw_group->pw_uid)) {
      return syscall_failed("Failed to set group id");
    }
  }
  if(pw_user) {
    if(setresuid(pw_user->pw_uid, pw_user->pw_uid, pw_user->pw_uid)) {
      return syscall_failed("Failed to set user id");
    }
  }

  /* Setup trace and execute jailed program. */
  ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  execvp(*argv, argv);
  return syscall_failed("execvp");
}

int main(int argc, char ** argv) {
  /* Read in arguments in first pass. */
  int argend = parse_arguments(argc, argv);
  if(argend == -1 || argend == argc) {
    print_usage(argv[0]);
    return -1;
  }

  /* If there is a configuration file read that in, but override config file
   * parameters with command line parameters. */
  if(!get_no_conf()) {
    parse_file(get_conf_file().c_str());
    parse_arguments(argc, argv);
  }

  if(get_help()) {
    print_usage(argv[0]);
    return 0;
  }

  init_process_state();
  if(!safemem_init()) {
    return syscall_failed("failed to init memory");
  }

  /* Fork and handle child process. */
  int pid_root = fork();
  if(pid_root == -1) {
    return syscall_failed("fork failed");
  } else if(pid_root == 0) {
    return sjail_child(argv + argend);
  }

  proc_data[pid_root] = pid_data(pid_root, true, 0U, create_root_filters());

  /* Initialize the report file. */
  if(!init_report()) {
    return teardown_processes("failed to open report file");
  }
  log_create(pid_root, getpid(), CREATE_ROOT);

  /* If a wall timer is set set an alarm to wake us up. */
  if(get_wall_time() != TIME_NO_LIMIT) {
    struct sigaction sigact;
    sigact.sa_handler = handle_sigalrm;
    sigact.sa_flags = SA_RESTART;
    sigfillset(&sigact.sa_mask);
    if(sigaction(SIGALRM, &sigact, NULL)) {
      return syscall_failed("sigaction");
    }
    alarm(get_wall_time());
  }

  /* Main trace loop. */
  for(bool firstTrace = true; ; firstTrace = false) {
    pid_t pid;
    int status;
    rusage resources;

    in_wait = true;
    if(alarm_fired) {
      teardown_processes(NULL);
      alarm_fired = false;
    }
    pid = wait3(&status, __WALL, &resources);
    in_wait = false;

    if(pid == -1) {
      syscall_failed("wait3");
      return teardown_processes(NULL);
    }

    auto it_pdata = proc_data.find(pid);
    if(it_pdata == proc_data.end()) {
      return teardown_processes("unexpected pid from wait");
    }
    pid_data& pdata = it_pdata->second;

    if(firstTrace) {
      /* The first time we wait for the child is on exit to execve.  After this
       * syscalls traps will have bit 15 set in the status due to TRACESYSGOOD.
       */
      status |= 1 << 15;
      ptrace(PTRACE_SETOPTIONS, pid, NULL,
             PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK |
             PTRACE_O_TRACECLONE | PTRACE_O_TRACESYSGOOD |
             PTRACE_O_TRACEEXEC);
      firstTrace = false;
    }

    if(WIFSIGNALED(status)) {
      exit_data data(EXIT_SIGNAL, &resources);
      data.signum = WTERMSIG(status);
      if(cleanup_process(pid, data)) {
        return 0;
      }
    } else if(WIFEXITED(status)) {
      exit_data data(EXIT_STATUS, &resources);
      data.status = WEXITSTATUS(status);
      if(cleanup_process(pid, data)) {
        return 0;
      }
    } else if(WIFSTOPPED(status)) {
      int sig = WSTOPSIG(status);
      if(sig == SIGTRAP && (
          (status >> 16 & 0xFFFF) == PTRACE_EVENT_FORK ||
          (status >> 16 & 0xFFFF) == PTRACE_EVENT_VFORK ||
          (status >> 16 & 0xFFFF) == PTRACE_EVENT_CLONE)) {
        pid_t child_pid;
        if(ptrace(PTRACE_GETEVENTMSG, pid, 0, &child_pid) == -1) {
          syscall_failed("ptrace(GETEVENTMSG)");
          return teardown_processes(NULL);
        } else if(MAX_PIDS <= (size_t)child_pid) {
          return teardown_processes("unexpected child pid from ptrace");
        } else if(proc_data.find(child_pid) != proc_data.end()) {
          return teardown_processes("already tracing child");
        } else try {
          if((status >> 16 & 0xFFFF) == PTRACE_EVENT_CLONE) {
            proc_data[child_pid] =
                pid_data(child_pid, false, pdata.safe_mem_base,
                         clone_filters(pdata.filters));
            pdata.filters = clone_filters(pdata.filters);
            log_create(child_pid, pid, CREATE_CLONE);
          } else {
            proc_data[child_pid] =
                pid_data(child_pid, false, pdata.safe_mem_base,
                         fork_filters(pdata.filters));
            log_create(child_pid, pid, CREATE_FORK);
          }
        } catch(std::bad_alloc) {
          log_error(pid, "out of state space");
          return teardown_processes("out of memory");
        }

        errno = 0;
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
      } else if(sig == (SIGTRAP | 0x80)) try {
        switch(filter_system_call(pdata)) {
          case FILTER_KILL_PID: {
            exit_data data(EXIT_KILLED, &resources);
            if(ptrace(PTRACE_KILL, pid, NULL, NULL)) {
              syscall_failed("ptrace kill");
              return teardown_processes(NULL);
            } else if(cleanup_process(pid, data)) {
              return 0;
            }
            errno = 0;
            break;
          } case FILTER_KILL_ALL: {
            return teardown_processes(NULL);
            break;
          } default: {
            errno = 0;
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
          }
        }
      } catch(std::bad_alloc) {
        log_error(pid, "out of state space");
        return teardown_processes("out of memory");
      } else {
        if (sig == SIGTRAP && (status >> 16 & 0xFFFF) == PTRACE_EVENT_EXEC) {
          sig = 0;
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, (void*)(intptr_t)sig);
        if(sig != 0) {
          log_info(pid, 2, "signal " + get_signal_name(sig) + " delivered");
        }
      }

      if(errno) {
        syscall_failed("ptrace resume failed");

        exit_data data(EXIT_KILLED, &resources);
        if(ptrace(PTRACE_KILL, pid, NULL, NULL)) {
          syscall_failed("ptrace kill");
          return teardown_processes(NULL);
        } else if(cleanup_process(pid, data)) {
          return 0;
        }
      }
    } else if(!WIFCONTINUED(status)) {
      return teardown_processes("unknown status from wait");
    }
  }

  return 1;
}
