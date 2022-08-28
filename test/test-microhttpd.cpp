#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <microhttpd.h>
#define PORT 8888

MHD_Result answer_to_connection (void *cls, struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method, const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size, void **con_cls)
{
    const char *page  = "<html><body>Hello, browser!</body></html>";
    struct MHD_Response *response;
    MHD_Result ret;
    response = MHD_create_response_from_buffer (strlen (page),
                                                (void*) page, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}

int main ()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_POLL_INTERNAL_THREAD, PORT, NULL, NULL,
                               &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) return 1;

    while (true) {
        char c = getchar();
        if (c == 'q')
            break;
    }
    printf("Stop MHD daemon\n");
    MHD_stop_daemon (daemon);
    return 0;
}