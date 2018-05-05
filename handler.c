/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Internal Declarations */
HTTPStatus handle_browse_request(Request *request);
HTTPStatus handle_file_request(Request *request);
HTTPStatus handle_cgi_request(Request *request);
HTTPStatus handle_error(Request *request, HTTPStatus status);

/**
 * Handle HTTP Request.
 *
 * @param   r           HTTP Request structure
 * @return  Status of the HTTP request.
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
HTTPStatus  handle_request(Request *r) {
    HTTPStatus result;
    struct stat st;

    /* Parse request */
    if (parse_request(r) < 0) {
        return handle_error(r, HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }

    /* Determine request path */
    if(determine_request_path(r->uri) == NULL)
    {
        log("Couldn't determine path of uri");

        return handle_error(r, HTTP_STATUS_NOT_FOUND);
    }


    r->path = strdup(determine_request_path(r->uri));
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type based on file type */
    if(stat(r->path, &st) < 0)
    {
        log("Stat didn't work: %s", strerror(errno));
        return handle_error(r, HTTP_STATUS_NOT_FOUND);
    }

    // CGI or static
    if(S_ISREG(st.st_mode))
    {
        // CGI
        if(st.st_mode & S_IXUSR)
        {
            result = handle_cgi_request(r);
        }
        // reg file
        else
        {
            result = handle_file_request(r);
        }
    }
    // dir
    else if(S_ISDIR(st.st_mode))
    {
        result = handle_browse_request(r);
    }
    // something else
    else
    {
        log("Unknown file type O_O");
        return handle_error(r, HTTP_STATUS_NOT_FOUND);
    }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    return result;
}

/**
 * Handle browse request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP browse request.
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
HTTPStatus  handle_browse_request(Request *r) {
    struct dirent *entries;
    DIR * dir;
    // What the heck is this used for????
    //int n;

    debug("Browsing directory");

    /* Open a directory for reading or scanning */
    if((dir = opendir(r->path)) == NULL)
    {
        log("Couldn't open directory: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    /* Write HTTP Header with OK Status and text/html Content-Type */
    
    fprintf(r->file, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");

    /* For each entry in directory, emit HTML list item */

    fprintf(r->file, "<html>\r\n");
    fprintf(r->file, "<ul>\r\n");

    // loop while entry exists
    char link [BUFSIZ];
    char host [BUFSIZ];

    if(gethostname(host, BUFSIZ) < 0)
    {
        log("Couldn't resolve hostname: %s\n", strerror(errno));
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    while((entries = readdir(dir)) != NULL)
    {
        if(streq(entries->d_name, ".")) continue;

        // make link
        strcpy(link, "http://");
        strcat(link, host);
        strcat(link, ":");
        strcat(link, Port);
        strcat(link, r->uri);
        strcat(link, "/");
        strcat(link, entries->d_name);

        log("Link: %s", link);
        fprintf(r->file, "<li><a href=%s>%s</a></li>\r\n", link, entries->d_name);
    }

    fprintf(r->file, "</ul>\r\n");
    fprintf(r->file, "</html>\r\n");

    closedir(dir);

    /* Flush socket, return OK */
    fflush(r->file);
    return HTTP_STATUS_OK;
}

/**
 * Handle file request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
HTTPStatus  handle_file_request(Request *r) {
    FILE *fs = NULL;
    char buffer[BUFSIZ] = {0};
    char *mimetype = NULL;
    //size_t nread = 0;

    debug("Making some file stuff happen");

    /* Open file for reading */
    int rfd = open(r->path, O_RDONLY);
    if (rfd < 0)
    {
        log("Error reading file: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }
    fs = fdopen(rfd, "r");

    /* Determine mimetype */
    mimetype = determine_mimetype(r->uri);
    if(mimetype == NULL)
    {
        log("Cannot determine mimetype");
        goto fail;
    }

    log("Mimetype: %s", mimetype);

    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->file, "HTML/1.1 200 OK\nContent-Type: %s\n\r\n", mimetype);
    fprintf(stderr, "HTML/1.1 200 OK\nContent-Type: %s\n\r\n", mimetype);

    /* Read from file and write to socket in chunks */
    while((fgets(buffer, BUFSIZ, fs)) != NULL)
    {
        fputs(buffer, r->file);
        fputs(buffer, stderr);
    }

    /* Close file, flush socket, deallocate mimetype, return OK */
    fclose(fs);
    fflush(r->file);
    free(mimetype);

    return HTTP_STATUS_OK;

fail:
    /* Close file, free mimetype, return INTERNAL_SERVER_ERROR */
    if(rfd != -1) close(rfd);
    if(mimetype != NULL) free(mimetype);

    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

/**
 * Handle CGI request
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
HTTPStatus handle_cgi_request(Request *r) {
    FILE *pfs = NULL;
    char buffer[BUFSIZ] = {0};

    debug("Making some CGI happen");

    /* Export CGI environment variables from request structure:
     * http://en.wikipedia.org/wiki/Common_Gateway_Interface */
    // DOCUMENT_ROOT
    setenv("DOCUMENT_ROOT", RootPath, 1);

    // QUERY_STRING
    setenv("QUERY_STRING", r->query, 1);

    // REMOTE_ADDR
    setenv("REMOTE_ADDR", r->host, 1);

    // REMOTE_PORT
    setenv("REMOTE_PORT", r->port, 1);

    // REQUEST_METHOD
    setenv("REQUEST_METHOD", r->method, 1);

    // REQUEST_URI
    setenv("REQUEST_URI", r->uri, 1);

    // SCRIPT_NAME
    setenv("SCRIPT_NAME", r->path, 1);

    // SERVER_PORT
    setenv("SERVER_PORT", Port, 1);

    /* Export CGI environment variables from request headers */
    Header * curr = r->headers;

    while(curr != NULL)
    {
        if(streq(curr->name, "Host") == 0)
        {
            setenv("HTTP_HOST", curr->value, 1);
        }
        else if(streq(curr->name, "Accept") == 0)
        {
            setenv("HTTP_ACCEPT", curr->value, 1);
        }
        else if(streq(curr->name, "Accept-Language") == 0)
        {
            setenv("HTTP_ACCEPT_LANGUAGE", curr->value, 1);
        }
        else if(streq(curr->name, "Accept-Encoding") == 0)
        {
            setenv("HTTP_ACCEPT_ENCODING", curr->value, 1);
        }
        else if(streq(curr->name, "Connection") == 0)
        {
            setenv("HTTP_CONNECTION", curr->value, 1);
        }
        else if(streq(curr->name, "User-Agent") == 0)
        {
            setenv("HTTP_USER_AGENT", curr->value, 1);
        }
        curr = curr->next;
    }

    /* POpen CGI Script */
    pfs = popen(r->path, "r");

    if(pfs < 0)
    {
        log("Couldn't open cgi script: %s\n", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    /* Copy data from popen to socket */
    fgets(buffer, BUFSIZ, pfs);
    write(r->fd, buffer, BUFSIZ);

    /* Close popen, flush socket, return OK */
    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP error request.
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
HTTPStatus  handle_error(Request *r, HTTPStatus status) {
    debug("Handling error");

    const char *status_string = http_status_string(status);
    //int status_int = 0;


    /*switch(status)
    {
        case HTTP_STATUS_OK:
            status_int = 200;
            break;
        case HTTP_STATUS_BAD_REQUEST:
            status_int = 400;
            break;
        case HTTP_STATUS_NOT_FOUND:
            status_int = 404;
            break;
        case HTTP_STATUS_INTERNAL_SERVER_ERROR:
            status_int = 500;
            break;
        default:
            status_int = -1;
            break;
    }*/

    /* Write HTTP Header */

    fprintf(r->file, "HTTP/1.0 %s\r\n", status_string);

    /* Write HTML Description of Error*/
    fprintf(r->file, "\r\n");
    fprintf(r->file, "<html>\r\n");
    fprintf(r->file, "<h>%s</h>\r\n", status_string);
    fprintf(r->file, "</html>\r\n");

    /* Return specified status */
    fflush(r->file);
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
