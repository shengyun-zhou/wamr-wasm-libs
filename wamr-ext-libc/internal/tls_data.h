#pragma once
#include <unistd.h>
#include <errno.h>
#include <wasi/libc-find-relpath.h>

struct cxx_dtor_stacknode {
    struct cxx_dtor_stacknode* next_node;
    void (*dtor_func)(void*);
    void *arg;
};

struct pthread_cleanup_handler_stacknode;
struct pthread_routine_bundle;

struct tls_data {
    int tls_errno;
    int tls_h_errno;
    pid_t cache_tid;
    struct __wasilibc_find_path_tls_data wasi_find_path_data;
    struct pthread_cleanup_handler_stacknode* cleanup_handler_stacktop;
    struct cxx_dtor_stacknode* cxx_dtor_stacktop;
    struct pthread_routine_bundle* thread_routine_bundle;
    char thread_name[32];
};

extern _Thread_local struct tls_data __g_tls_data;
