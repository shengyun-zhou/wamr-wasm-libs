#pragma once

/* All private struct types used for extended imported WASI call MUST include this header */
/* For ABI backward and forward compatibility */

typedef struct wamr_wasi_struct_header {
    uint16_t struct_size;
} wamr_wasi_struct_header;

#define DEFINE_WAMR_WASI_STRUCT_VAR(struct_name, var_name) \
    struct struct_name var_name = {0}; (var_name)._s_header.struct_size = sizeof(var_name)
