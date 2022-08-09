#include <wamr_ext.h>
#include <stdint.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#define WAMR_EXT_MODULE "wamr_ext"

int32_t __imported_wamr_ext_sysctl(char*, void*, uint32_t*) __attribute__((
    __import_module__(WAMR_EXT_MODULE),
    __import_name__("wamr_ext_sysctl")
));

int wamr_ext_sysctl(const char* name, void* buf, unsigned int* buflen) {
    int err = __imported_wamr_ext_sysctl((char*)name, buf, buflen);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

#pragma clang diagnostic pop
