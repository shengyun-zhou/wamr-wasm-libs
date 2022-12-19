cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_SYSTEM_NAME WASI)
project(wamr_wasi_libc C)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Werror -Wno-parentheses -Wno-shift-op-parentheses -Wno-bitwise-op-parentheses -Wno-logical-op-parentheses)

set(WASI_LIBC_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/wasi-libc)
set(PREBUILT_WASI_LIBC ${WASI_LIBC_SOURCE}/sysroot/lib/wasm32-wasi-pthread/libc.a)
set(OUTPUT_WASI_LIBC ${CMAKE_CURRENT_SOURCE_DIR}/sysroot/lib/wasm32-wasi-pthread/libc.a)
set(WASI_LIBC_TOP_HALF ${WASI_LIBC_SOURCE}/libc-top-half/musl)
set(WASI_LIBC_TOP_HALF_SRC ${WASI_LIBC_TOP_HALF}/src)
set(WAMR_EXT_LIBC_ROOT_DIR wamr-ext-libc)
set(WAMR_EXT_LIBC_INCLUDE_DIR ${WAMR_EXT_LIBC_ROOT_DIR}/include)

set(_WAMR_EXT_LIBC_HEADERS
        net/if.h
        sys/wait.h
        sys/mount.h
        sys/socket.h
        sys/statvfs.h
        dlfcn.h
        ifaddrs.h
        netdb.h
        resolv.h
        spawn.h
        syslog.h
        wamr_ext.h
        )

set(TEST_ALL_INCLUDE_FILES "")
set(WAMR_EXT_LIBC_HEADERS "")
foreach(header_file IN ITEMS ${_WAMR_EXT_LIBC_HEADERS})
    list(APPEND WAMR_EXT_LIBC_HEADERS ${WAMR_EXT_LIBC_INCLUDE_DIR}/${header_file})
    string(APPEND TEST_ALL_INCLUDE_FILES "#include <${header_file}>\n")
endforeach()
configure_file(${WAMR_EXT_LIBC_ROOT_DIR}/include-all.c.in ${CMAKE_CURRENT_BINARY_DIR}/include-all.c)
add_library(wamr_libc_include_all OBJECT
        ${CMAKE_CURRENT_BINARY_DIR}/include-all.c)
target_include_directories(wamr_libc_include_all PRIVATE
        ${WAMR_EXT_LIBC_INCLUDE_DIR}
        ${WASI_LIBC_SOURCE}/sysroot/include
        )

add_custom_target(pre_wamr_libc
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${WASI_LIBC_SOURCE}/libc-top-half/musl/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WASI_LIBC_SOURCE}/libc-top-half/musl/include.bak ${WASI_LIBC_SOURCE}/libc-top-half/musl/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WAMR_EXT_LIBC_INCLUDE_DIR} ${WASI_LIBC_SOURCE}/libc-top-half/musl/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        SOURCES ${WAMR_EXT_LIBC_HEADERS}
        )
add_dependencies(pre_wamr_libc wamr_libc_include_all)

add_library(wamr_top_half OBJECT
        ${WASI_LIBC_TOP_HALF_SRC}/network/inet_addr.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/inet_legacy.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/inet_ntoa.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/send.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/recv.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/dn_expand.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/dns_parse.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/freeaddrinfo.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gai_strerror.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getaddrinfo.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyaddr_r.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyaddr.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyname_r.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyname.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyname2_r.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/gethostbyname2.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getnameinfo.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getservbyname_r.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getservbyname.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getservbyport_r.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/getservbyport.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/lookup_ipliteral.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/lookup_name.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/lookup_serv.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_init.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_mkquery.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_msend.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_query.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_querydomain.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_send.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/res_state.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/resolvconf.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/herror.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/hstrerror.c
        ${WASI_LIBC_TOP_HALF_SRC}/network/h_errno.c

        ${WASI_LIBC_TOP_HALF_SRC}/malloc/posix_memalign.c
        ${WASI_LIBC_TOP_HALF_SRC}/misc/realpath.c
        ${WASI_LIBC_TOP_HALF_SRC}/linux/gettid.c

        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawn_file_actions_init.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawn_file_actions_destroy.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawn_file_actions_addopen.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawn_file_actions_addclose.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawn_file_actions_adddup2.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_init.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_destroy.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_getflags.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_setflags.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_getpgroup.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_setpgroup.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_getsigdefault.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_setsigdefault.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_getsigmask.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_setsigmask.c
        ${WASI_LIBC_TOP_HALF_SRC}/process/posix_spawnattr_sched.c

        ${WASI_LIBC_TOP_HALF_SRC}/thread/pthread_atfork.c
        )
target_include_directories(wamr_top_half PRIVATE
        ${WASI_LIBC_TOP_HALF_SRC}/include
        ${WASI_LIBC_TOP_HALF_SRC}/internal
        ${WASI_LIBC_TOP_HALF_SRC}/../arch/wasm32
        ${WAMR_EXT_LIBC_INCLUDE_DIR}
        ${WASI_LIBC_SOURCE}/sysroot/include
        )
add_dependencies(wamr_top_half pre_wamr_libc)

set(WASI_LIBC_BOTTOM_HALF_SRC ${WASI_LIBC_SOURCE}/libc-bottom-half/source)
set(WASI_LIBC_BOTTOM_HALF_CLOUDLIBC_SRC ${WASI_LIBC_SOURCE}/libc-bottom-half/cloudlibc/src/libc)
add_library(wamr_libc STATIC
        $<TARGET_OBJECTS:wamr_top_half>
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/wamr_ext.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/sysconf.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/cxxabi.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/user_group.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/utility.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/filesystem.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/socket_api.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/signal.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/main.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/process.c
        ${WAMR_EXT_LIBC_ROOT_DIR}/src/malloc.c
        )
target_include_directories(wamr_libc PRIVATE
        ${WAMR_EXT_LIBC_INCLUDE_DIR}
        ${WASI_LIBC_TOP_HALF_SRC}/../arch/wasm32
        ${WASI_LIBC_SOURCE}/sysroot/include
        )

add_custom_command(TARGET wamr_libc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PREBUILT_WASI_LIBC} ${CMAKE_CURRENT_BINARY_DIR}/libc.a
        # Remove some original function implementation
        COMMAND ${CMAKE_AR} -d ${CMAKE_CURRENT_BINARY_DIR}/libc.a
        recv.o send.o getsockopt.o accept.o shutdown.o
        iconv.o iconv_close.o
        COMMAND ${CMAKE_AR} qscL ${CMAKE_CURRENT_BINARY_DIR}/libc.a $<TARGET_FILE:wamr_libc>
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libc.a ${OUTPUT_WASI_LIBC}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WAMR_EXT_LIBC_INCLUDE_DIR} sysroot/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
