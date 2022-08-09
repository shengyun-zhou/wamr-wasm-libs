#ifndef WAMR_EXT_H
#define WAMR_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

int wamr_ext_sysctl(const char* name, void* buf, unsigned int* buflen);

#ifdef __cplusplus
}
#endif

#endif
