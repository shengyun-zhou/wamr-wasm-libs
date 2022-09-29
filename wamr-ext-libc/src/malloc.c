#include <stdlib.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

void* __imported_malloc(uint32_t) __attribute__((
    __import_name__("malloc")
));

void *malloc(size_t s) {
    void* p = __imported_malloc(s);
    if (!p)
        errno = ENOMEM;
    return p;
}

void __imported_free(void*) __attribute__((
    __import_name__("free")
));

void free(void *ptr) {
    if (!ptr)
        return;
    __imported_free(ptr);
}

void* __imported_calloc(uint32_t, uint32_t) __attribute__((
    __import_name__("calloc")
));

void *calloc(size_t nmemb, size_t size) {
    void* p = __imported_calloc(nmemb, size);
    if (!p)
        errno = ENOMEM;
    return p;
}

void* __imported_realloc(void*, uint32_t) __attribute__((
    __import_name__("realloc")
));

void *realloc(void *ptr, size_t size) {
    if (!ptr)
        return malloc(size);
    void* p = __imported_realloc(ptr, size);
    if (!p)
        errno = ENOMEM;
    return p;
}

void* __imported_aligned_alloc(uint32_t, uint32_t) __attribute__((
    __import_name__("aligned_alloc")
));

void *aligned_alloc(size_t alignment, size_t size) {
    void* p = __imported_aligned_alloc(alignment, size);
    if (!p)
        errno = ENOMEM;
    return p;
}

void *__libc_malloc(size_t) __attribute__((alias("malloc")));
void __libc_free(void *) __attribute__((alias("free")));
void *__libc_calloc(size_t, size_t) __attribute__((alias("calloc")));
