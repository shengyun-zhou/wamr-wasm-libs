#pragma once
#include <unistd.h>
#include <errno.h>
#include <wasi/libc-find-relpath.h>

struct pthread_cleanup_handler_stacknode {
    struct pthread_cleanup_handler_stacknode* next_node;
    void (*routine)(void *);
    void *arg;
};

struct tls_data {
    int tls_errno;
    int tls_h_errno;
    pid_t cache_tid;
    struct __wasilibc_find_path_tls_data wasi_find_path_data;
    struct pthread_cleanup_handler_stacknode* cleanup_handler_stacktop;
};

extern _Thread_local struct tls_data __g_tls_data;
