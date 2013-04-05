#ifndef SYSCALL_TAB_H
#define SYSCALL_TAB_H

/* common syscalls */
typedef enum SYSCALL {
sys_none,

sys_accept,
sys_accept4,
sys_access,
sys_adjtimex,
sys_arch_prctl,
sys_bind,
sys_brk,
sys_capget,
sys_capset,
sys_chdir,
sys_chmod,
sys_chown,
sys_clock_adjtime,
sys_clock_gettime,
sys_clock_nanosleep,
sys_clock_settime,
sys_clone,
sys_close,
sys_connect,
sys_creat,
sys_create_module,
sys_dup2,
sys_dup3,
sys_epoll_create,
sys_epoll_create1,
sys_epoll_ctl,
sys_epoll_pwait,
sys_epoll_wait,
sys_eventfd,
sys_eventfd2,
sys_execve,
sys_exit,
sys_exit_group,
sys_faccessat,
sys_fadvise64,
sys_fadvise64_64,
sys_fallocate,
sys_fchmod,
sys_fchmodat,
sys_fchown,
sys_fchownat,
sys_fcntl,
sys_fgetxattr,
sys_flistxattr,
sys_flock,
sys_fork,
sys_fremovexattr,
sys_fsetxattr,
sys_fstat,
sys_fstat64,
sys_fstatfs,
sys_fstatfs64,
sys_ftruncate,
sys_ftruncate64,
sys_futex,
sys_futimesat,
sys_get_mempolicy,
sys_get_robust_list,
sys_get_thread_area,
sys_getcpu,
sys_getcwd,
sys_getdents,
sys_getdents64,
sys_getdtablesize,
sys_getgroups,
sys_getgroups32,
sys_gethostname,
sys_getitimer,
sys_getpeername,
sys_getpmsg,
sys_getpriority,
sys_getresuid,
sys_getrlimit,
sys_getrusage,
sys_getsockname,
sys_getsockopt,
sys_gettimeofday,
sys_getuid,
sys_getxattr,
sys_init_module,
sys_inotify_add_watch,
sys_inotify_init1,
sys_inotify_rm_watch,
sys_io_cancel,
sys_io_destroy,
sys_io_getevents,
sys_io_setup,
sys_io_submit,
sys_ioctl,
sys_ipc,
sys_kill,
sys_link,
sys_linkat,
sys_listen,
sys_listxattr,
sys_llseek,
sys_lseek,
sys_lstat,
sys_lstat64,
sys_madvise,
sys_mbind,
sys_migrate_pages,
sys_mincore,
sys_mkdir,
sys_mkdirat,
sys_mknod,
sys_mknodat,
sys_mlockall,
sys_mmap,
sys_modify_ldt,
sys_mount,
sys_move_pages,
sys_mprotect,
sys_mq_getsetattr,
sys_mq_notify,
sys_mq_open,
sys_mq_timedreceive,
sys_mq_timedsend,
sys_mremap,
sys_msgctl,
sys_msgget,
sys_msgrcv,
sys_msgsnd,
sys_msync,
sys_munmap,
sys_nanosleep,
sys_newfstatat,
sys_old_mmap,
sys_oldfstat,
sys_oldlstat,
sys_oldselect,
sys_oldstat,
sys_open,
sys_openat,
sys_personality,
sys_pipe,
sys_pipe2,
sys_poll,
sys_ppoll,
sys_prctl,
sys_pread,
sys_preadv,
sys_prlimit64,
sys_process_vm_readv,
sys_process_vm_writev,
sys_pselect6,
sys_ptrace,
sys_putpmsg,
sys_pwrite,
sys_pwritev,
sys_query_module,
sys_quotactl,
sys_read,
sys_readahead,
sys_readdir,
sys_readlink,
sys_readlinkat,
sys_readv,
sys_reboot,
sys_recv,
sys_recvfrom,
sys_recvmmsg,
sys_recvmsg,
sys_remap_file_pages,
sys_removexattr,
sys_renameat,
sys_restart_syscall,
sys_rt_sigaction,
sys_rt_sigpending,
sys_rt_sigprocmask,
sys_rt_sigqueueinfo,
sys_rt_sigsuspend,
sys_rt_sigtimedwait,
sys_rt_tgsigqueueinfo,
sys_sched_get_priority_min,
sys_sched_getaffinity,
sys_sched_getparam,
sys_sched_getscheduler,
sys_sched_rr_get_interval,
sys_sched_setaffinity,
sys_sched_setparam,
sys_sched_setscheduler,
sys_select,
sys_semctl,
sys_semget,
sys_semop,
sys_semtimedop,
sys_send,
sys_sendfile,
sys_sendfile64,
sys_sendmmsg,
sys_sendmsg,
sys_sendto,
sys_set_mempolicy,
sys_set_thread_area,
sys_setdomainname,
sys_setfsuid,
sys_setgroups,
sys_setgroups32,
sys_sethostname,
sys_setitimer,
sys_setpriority,
sys_setresuid,
sys_setreuid,
sys_setrlimit,
sys_setsockopt,
sys_settimeofday,
sys_setuid,
sys_setxattr,
sys_shmat,
sys_shmctl,
sys_shmdt,
sys_shmget,
sys_shutdown,
sys_sigaction,
sys_sigaltstack,
sys_siggetmask,
sys_signal,
sys_signalfd,
sys_signalfd4,
sys_sigpending,
sys_sigprocmask,
sys_sigreturn,
sys_sigsetmask,
sys_sigsuspend,
sys_socket,
sys_socketcall,
sys_socketpair,
sys_splice,
sys_stat,
sys_stat64,
sys_statfs,
sys_statfs64,
sys_stime,
sys_swapon,
sys_symlinkat,
sys_sysctl,
sys_sysinfo,
sys_syslog,
sys_tee,
sys_tgkill,
sys_time,
sys_timer_create,
sys_timer_gettime,
sys_timer_settime,
sys_timerfd,
sys_timerfd_create,
sys_timerfd_gettime,
sys_timerfd_settime,
sys_times,
sys_truncate,
sys_truncate64,
sys_umask,
sys_umount2,
sys_uname,
sys_unlinkat,
sys_unshare,
sys_utime,
sys_utimensat,
sys_utimes,
sys_vfork,
sys_vmsplice,
sys_wait4,
sys_waitid,
sys_waitpid,
sys_write,
sys_writev,

#ifdef X32
sys_lseek32,
#endif

sys_add_key,
sys_fanotify_init,
sys_fanotify_mark,
sys_ioperm,
sys_iopl,
sys_ioprio_get,
sys_ioprio_set,
sys_kexec_load,
sys_keyctl,
sys_lookup_dcookie,
sys_name_to_handle_at,
sys_open_by_handle_at,
sys_perf_event_open,
sys_request_key,
sys_sync_file_range,
sys_sysfs,
sys_vm86old,
sys_vm86,

sys_acct,
sys_chroot,
sys_clock_getres,
sys_delete_module,
sys_dup,
sys_fchdir,
sys_fdatasync,
sys_fsync,
sys_getegid,
sys_geteuid,
sys_getgid,
sys_getresgid,
sys_mlock,
sys_mq_unlink,
sys_munlock,
sys_pivotroot,
sys_rename,
sys_rmdir,
sys_sched_get_priority_max,
sys_set_robust_list,
sys_setfsgid,
sys_setgid,
sys_setns,
sys_setregid,
sys_setresgid,
sys_swapoff,
sys_symlink,
sys_syncfs,
sys_umount,
sys_unlink,
sys_uselib,

sys_getpgid,
sys_getpid,
sys_getppid,
sys_gettid,
sys_idle,
sys_inotify_init,
sys_munlockall,
sys_pause,
sys_rt_sigreturn,
sys_sched_yield,
sys_setsid,
sys_set_tid_address,
sys_setup,
sys_sync,
sys_timer_delete,
sys_timer_getoverrun,
sys_vhangup,

/*_lu/ld does the right thing */
sys_alarm,
sys_getpgrp,
sys_getsid,
sys_nice,
sys_setpgid,
sys_setpgrp,

/* unimplemented */
sys_afs_syscall,
sys_break,
sys_ftime,
sys_get_kernel_syms,
sys_gtty,
sys_lock,
sys_mpx,
sys_nfsservctl,
sys_phys,
sys_profil,
sys_prof,
sys_security,
sys_stty,
sys_tuxcall,
sys_ulimit,
sys_ustat,
sys_vserver,

/* deprecated */
sys_bdflush,
sys_oldolduname,
sys_olduname,

sys_oldepoll_ctl,
sys_oldepoll_wait,

sys_socket_subcall,
sys_ipc_subcall,

NUM_SYSCALLS
} SYSCALL;

#endif // SYSCALL_TAB_H
