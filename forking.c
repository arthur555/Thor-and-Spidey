/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>

/**
 * Fork incoming HTTP requests to handle the concurrently.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Exit status of server (EXIT_SUCCESS).
 *
 * The parent should accept a request and then fork off and let the child
 * handle the request.
 **/
int forking_server(int sfd) {
    /* Accept and handle HTTP request */
    if (sfd < 0){
        return EXIT_FAILURE;
    }
    while (true) {
    	/* Accept request */
         Request* client = NULL;
         if ((client = accept_request(sfd)) == NULL) {
             return EXIT_FAILURE;                       
         }
        //FILE* client_file = client->file;
        /*if (!client_file) {
            continue;
        }*/
	/* Ignore children */
        signal(SIGCHLD,SIG_IGN);
	/* Fork off child process to handle request */
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr,"Unable to fork:%s\n",strerror(errno));
            free_request(client);
            return EXIT_FAILURE;
            //fclose(client_file);
        } else if (pid==0) {
            debug("Handling client request");
            handle_request(client);
            free_request(client);
            return EXIT_SUCCESS;
                //fclose(client_file);
                //handle_error(client,handle_request(client));
             /*else {
                //handle_request(client);
                //fclose(client_file);
                exit(EXIT_SUCCESS);
            }*/
        } else {
            free_request(client);
        }
    }

    /* Close server socket */
    close(sfd);
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
