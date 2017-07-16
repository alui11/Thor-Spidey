/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time
 **/
void
single_server(int sfd)
{
    struct request *request;

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
        request = accept_request(sfd);

	/* Handle request */
        handle_request(request);
        //do you we need to use the return value of ^^?
        //Why while true?

	/* Free request */
        free_request(request);
    }

    /* Close socket and exit */
    close(sfd);
    exit(EXIT_SUCCESS);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
