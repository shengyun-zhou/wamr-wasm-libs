#include <string.h>
#include <stdint.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

void* __imported_memcpy(void*, const void*, uint32_t) __attribute__((
    __import_name__("memcpy")
));

void *memcpy(void *__restrict__ dst, const void *__restrict__ src, size_t n) {
    return __imported_memcpy(dst, src, n);
}

void* __imported_memmove(void*, const void*, uint32_t) __attribute__((
    __import_name__("memmove")
));

void *memmove(void *dst, const void *src, size_t n) {
    return __imported_memmove(dst, src, n);
}

void* __imported_memset(void*, int32_t, uint32_t) __attribute__((
    __import_name__("memset")
));

void *memset(void *dst, int c, size_t n) {
    return __imported_memset(dst, c, n);
}

int32_t __imported_memcmp(const void*, const void*, uint32_t) __attribute__((
    __import_name__("memcmp")
));

int memcmp(const void* s1, const void* s2, size_t n) {
    return __imported_memcmp(s1, s2, n);
}

#pragma clang diagnostic pop
