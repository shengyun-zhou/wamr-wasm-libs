#include <stdlib.h>

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

#pragma clang diagnostic pop
