#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>

#define OUTLEN 255

int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == (iconv_t) - 1) {
        perror("iconv_open()");
        return -1;
    }
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}

int u2g(char *inbuf, int inlen, char *outbuf, int outlen) {
    return code_convert("UTF-8", "GBK", inbuf, inlen, outbuf, outlen);
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    return code_convert("GBK", "UTF-8", inbuf, inlen, outbuf, outlen);
}

int main() {
    char in_utf8[] = "utf-8字符串";
    char in_gb2312[] = "gbk\xd7\xd6\xb7\xfb\xb4\xae";
    char out[OUTLEN] = {0};

    u2g(in_utf8, strlen(in_utf8), out, OUTLEN);
    printf("utf-8-->gbk strlen(out)=%lu, out=%s\n", strlen(out), out);
    memset(out, 0, sizeof(out));
    g2u(in_gb2312, strlen(in_gb2312), out, OUTLEN);
    printf("gbk-->utf-8 strlen(out)=%lu, out=%s\n", strlen(out), out);
}
