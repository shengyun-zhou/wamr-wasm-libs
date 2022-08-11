#include <stdlib.h>
#include "../internal/tls_data.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
void *__cxa_allocate_exception(size_t thrown_size) {
    return malloc(thrown_size);
}

void __imported_cxa_throw(void*, void*, void(*)(void*)) __attribute__((
    __import_name__("__cxa_throw")
));

void __cxa_throw(void *thrown_exception, void* tinfo,
                 void (*dest)(void*)) {
    __imported_cxa_throw(thrown_exception, tinfo, dest);
}

int __cxa_thread_atexit_impl(void(*func)(void*), void *arg, void *dso_handle) {
    struct cxx_dtor_stacknode* new_node = malloc(sizeof(struct cxx_dtor_stacknode));
    new_node->next_node = __g_tls_data.cxx_dtor_stacktop;
    new_node->dtor_func = func;
    new_node->arg = arg;
    __g_tls_data.cxx_dtor_stacktop = new_node;
    return 0;
}

#pragma clang diagnostic pop
