// Source: https://github.com/util-linux/util-linux/blob/v2.38.1/libuuid/src/test_uuid.c
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <uuid/uuid.h>

static int test_uuid(const char *uuid, int isValid) {
    static const char *validStr[2] = {"invalid", "valid"};
    uuid_t uuidBits;
    int parsedOk;

    parsedOk = uuid_parse(uuid, uuidBits) == 0;

    printf("%s is %s", uuid, validStr[isValid]);
    if (parsedOk != isValid) {
        printf(" but uuid_parse says %s\n", validStr[parsedOk]);
        return 1;
    }
    printf(", OK\n");
    return 0;
}

int main(int argc, char **argv) {
    int failed = 0;

    failed += test_uuid("84949cc5-4701-4a84-895b-354c584a981b", 1);
    failed += test_uuid("84949CC5-4701-4A84-895B-354C584A981B", 1);
    failed += test_uuid("84949cc5-4701-4a84-895b-354c584a981bc", 0);
    failed += test_uuid("84949cc5-4701-4a84-895b-354c584a981", 0);
    failed += test_uuid("84949cc5x4701-4a84-895b-354c584a981b", 0);
    failed += test_uuid("84949cc504701-4a84-895b-354c584a981b", 0);
    failed += test_uuid("84949cc5-470104a84-895b-354c584a981b", 0);
    failed += test_uuid("84949cc5-4701-4a840895b-354c584a981b", 0);
    failed += test_uuid("84949cc5-4701-4a84-895b0354c584a981b", 0);
    failed += test_uuid("g4949cc5-4701-4a84-895b-354c584a981b", 0);
    failed += test_uuid("84949cc5-4701-4a84-895b-354c584a981g", 0);
    failed += test_uuid("00000000-0000-0000-0000-000000000000", 1);
    failed += test_uuid("01234567-89ab-cdef-0134-567890abcedf", 1);
    failed += test_uuid("ffffffff-ffff-ffff-ffff-ffffffffffff", 1);

    if (failed) {
        printf("%d failures.\n", failed);
        exit(1);
    }
    return 0;
}
