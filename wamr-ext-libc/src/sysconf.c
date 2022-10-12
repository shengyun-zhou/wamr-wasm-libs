#include <unistd.h>
#include <wamr_ext.h>
#include <limits.h>
#include <errno.h>

long __wamr_ext_sysconf(int name) {
    switch (name) {
        case _SC_NPROCESSORS_CONF:
        case _SC_NPROCESSORS_ONLN: {
            uint32_t cpu_count = 1;
            uint32_t buflen = sizeof(cpu_count);
            if (wamr_ext_sysctl("sysinfo.cpu_count", &cpu_count, &buflen) == 0)
                return cpu_count;
            return 1;
        }
        case _SC_PHYS_PAGES: {
            uint64_t mem_total = 0;
            uint32_t buflen = sizeof(mem_total);
            if (wamr_ext_sysctl("sysinfo.vm_mem_total", &mem_total, &buflen) == 0)
                return mem_total / PAGE_SIZE;
            return 0;
        }
        case _SC_AVPHYS_PAGES: {
            uint64_t mem_avail = 0;
            uint32_t buflen = sizeof(mem_avail);
            if (wamr_ext_sysctl("sysinfo.vm_mem_avail", &mem_avail, &buflen) == 0)
                return mem_avail / PAGE_SIZE;
            return 0;
        }
    }
    errno = EINVAL;
    return -1;
}
