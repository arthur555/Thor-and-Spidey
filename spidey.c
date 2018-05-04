/* spidey: Simple HTTP Server */

#include "spidey.h"
#include <linux/limits.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

/* Global Variables */
char *Port	      = "9898";
char *MimeTypesPath   = "/etc/mime.types";
char *DefaultMimeType = "text/plain";
char *RootPath	      = "www";

/**
 * Display usage message and exit with specified status code.
 *
 * @param   progname    Program Name
 * @param   status      Exit status.
 */
void usage(const char *progname, int status) {
    fprintf(stderr, "Usage: %s [hcmMpr]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -h            Display help message\n");
    fprintf(stderr, "    -c mode       Single or Forking mode\n");
    fprintf(stderr, "    -m path       Path to mimetypes file\n");
    fprintf(stderr, "    -M mimetype   Default mimetype\n");
    fprintf(stderr, "    -p port       Port to listen on\n");
    fprintf(stderr, "    -r path       Root directory\n");
    exit(status);
}

/**
 * Parse command-line options.
 *
 * @param   argc        Number of arguments.
 * @param   argv        Array of argument strings.
 * @param   mode        Pointer to ServerMode variable.
 * @return  true if parsing was successful, false if there was an error.
 *
 * This should set the mode, MimeTypesPath, DefaultMimeType, Port, and RootPath
 * if specified.
 */
bool parse_options(int argc, char *argv[], ServerMode *mode) {
    char* progname = argv[0];
    int argind = 1;
    if (argc==1) {
        usage(progname,0);
    }
    while (argind < argc && argv[argind][0]=='-') {
        switch(argv[argind][1]){
            case 'h': 
                usage(progname,0);
                break;
            case 'c':
                argind++;
                if (strcmp(argv[argind],"forking")==0) {
                    *mode = FORKING;
                } else if (strcmp(argv[argind],"single")==0){
                    *mode = SINGLE;
                } else {
                    *mode = UNKNOWN;
                }
                break;
            case 'm':
                argind++;
                MimeTypesPath = argv[argind];
                break;
            case 'M':
                argind++;
                DefaultMimeType = argv[argind];
                break;
            case 'p':
                argind++;
                Port = argv[argind];
                break;
            case 'r':
                argind++;
                RootPath = argv[argind];
                break;
            default:
                return false;
        }
        argind++;
    }
    return true;
}

/**
 * Parses command line options and starts appropriate server
 **/
int main(int argc, char *argv[]) {
    ServerMode mode;
    int sfd = 0;
    /* Parse command line options */
    if (parse_options(argc,argv,&mode)==false){
        return EXIT_FAILURE;
    }
    else {
        //parse_options(argc,argv,&mode);
    /* Listen to server socket */
        sfd = socket_listen(Port);
    /* Determine real RootPath */
        char buffer[PATH_MAX+1];
        RootPath = realpath(RootPath,buffer);
    }

    log("Listening on port %s", Port);
    debug("RootPath        = %s", RootPath);
    debug("MimeTypesPath   = %s", MimeTypesPath);
    debug("DefaultMimeType = %s", DefaultMimeType);
    debug("ConcurrencyMode = %s", mode == SINGLE ? "Single" : "Forking");

    /* Start either forking or single HTTP server */
    //sfd = socket_listen(Port);
    if (mode==FORKING) {
        if (forking_server(sfd)!=0){
            return EXIT_FAILURE;
        } /*else {
            forking_server(sfd);
        }*/
    } else if (mode==SINGLE) {
        if (single_server(sfd)!=0) {
            return EXIT_FAILURE;
        } /*else {
            single(sfd);
        }*/
    } else {
        if (forking_server(sfd)!=0) {
            return EXIT_FAILURE;
        } /*else {
            forking_server(sfd);
        }*/
    }
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
