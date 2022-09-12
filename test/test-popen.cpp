#include <stdio.h>
#include <string.h>
#include <errno.h>

int main() {
    const char* cmdline = "uname -a";
    FILE* pin = popen(cmdline, "r");
    if (pin) {
        char output[4096] = {0};
        fread(output, 1, sizeof(output), pin);
        printf("Command %s output: %s", cmdline, output);
        pclose(pin);
    } else {
        printf("Failed to exec command %s: %s\n", cmdline, strerror(errno));
        return 1;
    }
    return 0;
}
