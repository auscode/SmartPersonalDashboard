#pragma once

struct SyscallInfo {
  const char *name;
  const char *category; // "file", "network", "process", "memory", "signal", "other"
};

inline SyscallInfo getSyscallInfo(int num) {
  static const SyscallInfo table[] = {
    {"read", "file"}, // 0
    {"write", "file"}, // 1
    {"open", "file"}, // 2
    {"close", "file"}, // 3
    {"stat", "file"}, // 4
    {"fstat", "file"}, // 5
    {"lstat", "file"}, // 6
    {"poll", "other"}, // 7
    {"lseek", "file"}, // 8
    {"mmap", "memory"}, // 9
    {"mprotect", "memory"}, // 10
    {"munmap", "memory"}, // 11
    {"brk", "memory"}, // 12
    {"rt_sigaction", "signal"}, // 13
    {"rt_sigprocmask", "signal"}, // 14
    {"rt_sigreturn", "signal"}, // 15
    {"ioctl", "other"}, // 16
    {"pread64", "file"}, // 17
    {"pwrite64", "file"}, // 18
    {"readv", "file"}, // 19
    {"writev", "file"}, // 20
    {"access", "file"}, // 21
    {"pipe", "file"}, // 22
    {"select", "other"}, // 23
    {"sched_yield", "process"}, // 24
    {"mremap", "memory"}, // 25
    {"msync", "memory"}, // 26
    {"mincore", "memory"}, // 27
    {"madvise", "memory"}, // 28
    {"shmget", "memory"}, // 29
    {"shmat", "memory"}, // 30
    {"shmctl", "memory"}, // 31
    {"dup", "file"}, // 32
    {"dup2", "file"}, // 33
    {"pause", "signal"}, // 34
    {"nanosleep", "other"}, // 35
    {"getitimer", "other"}, // 36
    {"alarm", "other"}, // 37
    {"setitimer", "other"}, // 38
    {"getpid", "process"}, // 39
    {"sendfile", "file"}, // 40
    {"socket", "network"}, // 41
    {"connect", "network"}, // 42
    {"accept", "network"}, // 43
    {"sendto", "network"}, // 44
    {"recvfrom", "network"}, // 45
    {"sendmsg", "network"}, // 46
    {"recvmsg", "network"}, // 47
    {"shutdown", "network"}, // 48
    {"bind", "network"}, // 49
    {"listen", "network"}, // 50
    {"getsockname", "network"}, // 51
    {"getpeername", "network"}, // 52
    {"socketpair", "network"}, // 53
    {"setsockopt", "network"}, // 54
    {"getsockopt", "network"}, // 55
    {"clone", "process"}, // 56
    {"fork", "process"}, // 57
    {"vfork", "process"}, // 58
    {"execve", "process"}, // 59
    {"exit", "process"}, // 60
    {"wait4", "process"}, // 61
    {"kill", "process"}, // 62
    {"uname", "other"}, // 63
    {"semget", "other"}, // 64
    {"semop", "other"}, // 65
    {"semctl", "other"}, // 66
    {"shmdt", "memory"}, // 67
    {"msgget", "other"}, // 68
    {"msgsnd", "other"}, // 69
    {"msgrcv", "other"}, // 70
    {"msgctl", "other"}, // 71
    {"fcntl", "file"}, // 72
    {"flock", "file"}, // 73
    {"fsync", "file"}, // 74
    {"fdatasync", "file"}, // 75
    {"truncate", "file"}, // 76
    {"ftruncate", "file"}, // 77
    {"getdents", "file"}, // 78
    {"getcwd", "file"}, // 79
    {"chdir", "file"}, // 80
    {"fchdir", "file"}, // 81
    {"rename", "file"}, // 82
    {"mkdir", "file"}, // 83
    {"rmdir", "file"}, // 84
    {"creat", "file"}, // 85
    {"link", "file"}, // 86
    {"unlink", "file"}, // 87
    {"symlink", "file"}, // 88
    {"readlink", "file"}, // 89
    {"chmod", "file"}, // 90
    {"fchmod", "file"}, // 91
    {"chown", "file"}, // 92
    {"fchown", "file"}, // 93
    {"lchown", "file"}, // 94
    {"umask", "file"}, // 95
    {"gettimeofday", "other"}, // 96
    {"getrlimit", "other"}, // 97
    {"getrusage", "other"}, // 98
    {"sysinfo", "other"}, // 99
    {"times", "other"}, // 100
    {"ptrace", "process"}, // 101
    {"getuid", "other"}, // 102
    {"syslog", "other"}, // 103
    {"getgid", "other"}, // 104
    {"setuid", "other"}, // 105
    {"setgid", "other"}, // 106
    {"geteuid", "other"}, // 107
    {"getegid", "other"}, // 108
    {"setpgid", "process"}, // 109
    {"getppid", "process"}, // 110
    {"getpgrp", "process"}, // 111
    {"setsid", "process"}, // 112
    {"setreuid", "other"}, // 113
    {"setregid", "other"}, // 114
    {"getgroups", "other"}, // 115
    {"setgroups", "other"}, // 116
    {"setresuid", "other"}, // 117
    {"getresuid", "other"}, // 118
    {"setresgid", "other"}, // 119
    {"getresgid", "other"}, // 120
    {"getpgid", "process"}, // 121
    {"setfsuid", "other"}, // 122
    {"setfsgid", "other"}, // 123
    {"getsid", "process"}, // 124
    {"capget", "other"}, // 125
    {"capset", "other"}, // 126
    {"rt_sigpending", "signal"}, // 127
    {"rt_sigtimedwait", "signal"}, // 128
    {"rt_sigqueueinfo", "signal"}, // 129
    {"rt_sigsuspend", "signal"}, // 130
    {"sigaltstack", "signal"}, // 131
    {"utime", "file"}, // 132
    {"mknod", "file"}, // 133
    {"uselib", "other"}, // 134
    {"personality", "process"}, // 135
    {"ustat", "other"}, // 136
    {"statfs", "file"}, // 137
    {"fstatfs", "file"}, // 138
    {"sysfs", "other"}, // 139
    {"getpriority", "other"}, // 140
    {"setpriority", "other"}, // 141
    {"sched_setparam", "process"}, // 142
    {"sched_getparam", "process"}, // 143
    {"sched_setscheduler", "process"}, // 144
    {"sched_getscheduler", "process"}, // 145
    {"sched_get_priority_max", "process"}, // 146
    {"sched_get_priority_min", "process"}, // 147
    {"sched_rr_get_interval", "process"}, // 148
    {"mlock", "memory"}, // 149
    {"munlock", "memory"}, // 150
    {"mlockall", "memory"}, // 151
    {"munlockall", "memory"}, // 152
    {"vhangup", "other"}, // 153
    {"modify_ldt", "other"}, // 154
    {"pivot_root", "file"}, // 155
    {"_sysctl", "other"}, // 156
    {"prctl", "process"}, // 157
    {"arch_prctl", "process"}, // 158
    {"adjtimex", "other"}, // 159
    {"setrlimit", "other"}, // 160
    {"chroot", "file"}, // 161
    {"sync", "file"}, // 162
    {"acct", "other"}, // 163
    {"settimeofday", "other"}, // 164
    {"mount", "file"}, // 165
    {"umount2", "file"}, // 166
    {"swapon", "file"}, // 167
    {"swapoff", "file"}, // 168
    {"reboot", "process"}, // 169
    {"sethostname", "other"}, // 170
    {"setdomainname", "other"}, // 171
    {"iopl", "other"}, // 172
    {"ioperm", "other"}, // 173
    {"create_module", "other"}, // 174
    {"init_module", "other"}, // 175
    {"delete_module", "other"}, // 176
    {"get_kernel_syms", "other"}, // 177
    {"query_module", "other"}, // 178
    {"quotactl", "file"}, // 179
    {"nfsservctl", "other"}, // 180
    {"getpmsg", "other"}, // 181
    {"putpmsg", "other"}, // 182
    {"afs_syscall", "other"}, // 183
    {"tuxcall", "other"}, // 184
    {"security", "other"}, // 185
    {"gettid", "process"}, // 186
    {"readahead", "file"}, // 187
    {"setxattr", "file"}, // 188
    {"lsetxattr", "file"}, // 189
    {"fsetxattr", "file"}, // 190
    {"getxattr", "file"}, // 191
    {"lgetxattr", "file"}, // 192
    {"fgetxattr", "file"}, // 193
    {"listxattr", "file"}, // 194
    {"llistxattr", "file"}, // 195
    {"flistxattr", "file"}, // 196
    {"removexattr", "file"}, // 197
    {"lremovexattr", "file"}, // 198
    {"fremovexattr", "file"}, // 199
    {"tkill", "process"}, // 200
    {"time", "other"}, // 201
    {"futex", "other"}, // 202
    {"sched_setaffinity", "process"}, // 203
    {"sched_getaffinity", "process"}, // 204
    {"set_thread_area", "other"}, // 205
    {"io_setup", "other"}, // 206
    {"io_destroy", "other"}, // 207
    {"io_getevents", "other"}, // 208
    {"io_submit", "other"}, // 209
    {"io_cancel", "other"}, // 210
    {"get_thread_area", "other"}, // 211
    {"lookup_dcookie", "other"}, // 212
    {"epoll_create", "other"}, // 213
    {"epoll_ctl_old", "other"}, // 214
    {"epoll_wait_old", "other"}, // 215
    {"remap_file_pages", "memory"}, // 216
    {"getdents64", "file"}, // 217
    {"set_tid_address", "process"}, // 218
    {"restart_syscall", "process"}, // 219
    {"semtimedop", "other"}, // 220
    {"fadvise64", "file"}, // 221
    {"timer_create", "other"}, // 222
    {"timer_settime", "other"}, // 223
    {"timer_gettime", "other"}, // 224
    {"timer_getoverrun", "other"}, // 225
    {"timer_delete", "other"}, // 226
    {"clock_settime", "other"}, // 227
    {"clock_gettime", "other"}, // 228
    {"clock_getres", "other"}, // 229
    {"clock_nanosleep", "other"}, // 230
    {"exit_group", "process"}, // 231
    {"epoll_wait", "other"}, // 232
    {"epoll_ctl", "other"}, // 233
    {"tgkill", "process"}, // 234
    {"utimes", "file"}, // 235
    {"vserver", "other"}, // 236
    {"mbind", "memory"}, // 237
    {"set_mempolicy", "memory"}, // 238
    {"get_mempolicy", "memory"}, // 239
    {"mq_open", "other"}, // 240
    {"mq_unlink", "other"}, // 241
    {"mq_timedsend", "other"}, // 242
    {"mq_timedreceive", "other"}, // 243
    {"mq_notify", "other"}, // 244
    {"mq_getsetattr", "other"}, // 245
    {"kexec_load", "process"}, // 246
    {"waitid", "process"}, // 247
    {"add_key", "other"}, // 248
    {"request_key", "other"}, // 249
    {"keyctl", "other"}, // 250
    {"ioprio_set", "other"}, // 251
    {"ioprio_get", "other"}, // 252
    {"inotify_init", "file"}, // 253
    {"inotify_add_watch", "file"}, // 254
    {"inotify_rm_watch", "file"}, // 255
    {"migrate_pages", "memory"}, // 256
    {"openat", "file"}, // 257
    {"mkdirat", "file"}, // 258
    {"mknodat", "file"}, // 259
    {"fchownat", "file"}, // 260
    {"futimesat", "file"}, // 261
    {"newfstatat", "file"}, // 262
    {"unlinkat", "file"}, // 263
    {"renameat", "file"}, // 264
    {"linkat", "file"}, // 265
    {"symlinkat", "file"}, // 266
    {"readlinkat", "file"}, // 267
    {"fchmodat", "file"}, // 268
    {"faccessat", "file"}, // 269
    {"pselect6", "other"}, // 270
    {"ppoll", "other"}, // 271
    {"unshare", "process"}, // 272
    {"set_robust_list", "process"}, // 273
    {"get_robust_list", "process"}, // 274
    {"splice", "file"}, // 275
    {"tee", "file"}, // 276
    {"sync_file_range", "file"}, // 277
    {"vmsplice", "file"}, // 278
    {"move_pages", "memory"}, // 279
    {"utimensat", "file"}, // 280
    {"epoll_pwait", "other"}, // 281
    {"signalfd", "signal"}, // 282
    {"timerfd_create", "other"}, // 283
    {"eventfd", "other"}, // 284
    {"fallocate", "file"}, // 285
    {"timerfd_settime", "other"}, // 286
    {"timerfd_gettime", "other"}, // 287
    {"accept4", "network"}, // 288
    {"signalfd4", "signal"}, // 289
    {"eventfd2", "other"}, // 290
    {"epoll_create1", "other"}, // 291
    {"dup3", "file"}, // 292
    {"pipe2", "file"}, // 293
    {"inotify_init1", "file"}, // 294
    {"preadv", "file"}, // 295
    {"pwritev", "file"}, // 296
    {"rt_tgsigqueueinfo", "signal"}, // 297
    {"perf_event_open", "other"}, // 298
    {"recvmmsg", "network"}, // 299
    {"fanotify_init", "file"}, // 300
    {"fanotify_mark", "file"}, // 301
    {"prlimit64", "other"}, // 302
    {"name_to_handle_at", "file"}, // 303
    {"open_by_handle_at", "file"}, // 304
    {"clock_adjtime", "other"}, // 305
    {"syncfs", "file"}, // 306
    {"sendmmsg", "network"}, // 307
    {"setns", "process"}, // 308
    {"getcpu", "other"}, // 309
    {"process_vm_readv", "process"}, // 310
    {"process_vm_writev", "process"}, // 311
    {"kcmp", "process"}, // 312
    {"finit_module", "other"}, // 313
    {"sched_setattr", "process"}, // 314
    {"sched_getattr", "process"}, // 315
    {"renameat2", "file"}, // 316
    {"seccomp", "other"}, // 317
    {"getrandom", "other"}, // 318
    {"memfd_create", "memory"}, // 319
    {"kexec_file_load", "process"}, // 320
    {"bpf", "other"}, // 321
    {"execveat", "process"}, // 322
    {"userfaultfd", "other"}, // 323
    {"membarrier", "other"}, // 324
    {"mlock2", "memory"}, // 325
    {"copy_file_range", "file"}, // 326
    {"preadv2", "file"}, // 327
    {"pwritev2", "file"}, // 328
    {"pkey_mprotect", "memory"}, // 329
    {"pkey_alloc", "memory"}, // 330
    {"pkey_free", "memory"}, // 331
    {"statx", "file"}, // 332
    {"io_pgetevents", "other"}, // 333
    {"rseq", "other"}, // 334
  };

  int size = static_cast<int>(sizeof(table) / sizeof(table[0]));
  if (num >= 0 && num < size) {
    return table[num];
  }

  // Sparse check for newer syscalls
  switch (num) {
    case 424: return {"pidfd_send_signal", "signal"};
    case 425: return {"io_uring_setup", "other"};
    case 426: return {"io_uring_enter", "other"};
    case 427: return {"io_uring_register", "other"};
    case 428: return {"open_tree", "file"};
    case 429: return {"move_mount", "file"};
    case 430: return {"fsopen", "file"};
    case 431: return {"fsconfig", "file"};
    case 432: return {"fsmount", "file"};
    case 433: return {"fspick", "file"};
    case 434: return {"pidfd_open", "process"};
    case 435: return {"clone3", "process"};
    case 436: return {"close_range", "file"};
    case 437: return {"openat2", "file"};
    case 438: return {"pidfd_getfd", "process"};
    case 439: return {"faccessat2", "file"};
    case 440: return {"process_madvise", "memory"};
    case 441: return {"epoll_pwait2", "other"};
    case 442: return {"mount_setattr", "file"};
    case 443: return {"quotactl_fd", "file"};
    case 444: return {"landlock_create_ruleset", "other"};
    case 445: return {"landlock_add_rule", "other"};
    case 446: return {"landlock_restrict_self", "other"};
    case 447: return {"memfd_secret", "memory"};
    case 448: return {"process_mrelease", "process"};
    case 449: return {"futex_waitv", "other"};
    case 450: return {"set_mempolicy_home_node", "memory"};
    case 451: return {"cachestat", "file"};
    case 452: return {"fchmodat2", "file"};
    case 453: return {"map_shadow_stack", "memory"};
    case 454: return {"futex_wake", "other"};
    case 455: return {"futex_wait", "other"};
    case 456: return {"futex_requeue", "other"};
    case 457: return {"statmount", "file"};
    case 458: return {"listmount", "file"};
    case 459: return {"lsm_get_self_attr", "other"};
    case 460: return {"lsm_set_self_attr", "other"};
    case 461: return {"lsm_list_modules", "other"};
  }

  return {"unknown", "other"};
}
