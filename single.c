/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Exit status of server (EXIT_SUCCESS).
 **/
int single_server(int sfd) {
    /* Accept and handle HTTP request */
    if (sfd < 0) {
        return EXIT_FAILURE;
    }
    while (true) {
    	/* Accept request */
        Request* client = NULL;
        if ((client = accept_request(sfd)) == NULL) {
            log("Accept_request failed!");
            return EXIT_FAILURE;
        }

        /* Handle request */
        debug("Accept success!");
        handle_request(client);
        debug("Handle success!");

	/* Free request */
        free_request(client);
    }

    /* Close server socket */
    close(sfd);
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
