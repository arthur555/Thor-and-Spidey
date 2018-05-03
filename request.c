/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

int parse_request_method(Request *r);
int parse_request_headers(Request *r);

/**
 * Accept request from server socket.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Newly allocated Request structure.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
Request * accept_request(int sfd) {
    Request *r;
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Allocate request struct (zeroed) */
    if((r = calloc(1, sizeof(Request))) == NULL)
    {
        log("Couldn't allocate memory: %s\n", strerror(errno));
        goto fail;
    }

    /* Accept a client */
    r->fd = accept(sfd, &raddr, &rlen);
    if(r->fd < 0)
    {
        log("accept failed: %s\n", strerror(errno));
        goto fail;
    }

    /* Lookup client information */
    if((getnameinfo(&raddr, rlen, r->host, NI_MAXHOST, r->port, NI_MAXHOST, 0)) < 0)
    {
        log("Failed to get client info: %s\n", strerror(errno));
        goto fail;
    }

    /* Open socket stream */
    r->file = fdopen(r->fd, "w+");
    if(!r->file)
    {
        log("Couldn't open stream: %s\n", strerror(errno));
        goto fail;
    }

    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    /* Deallocate request struct */
    free_request(r);

    return NULL;
}

/**
 * Deallocate request struct.
 *
 * @param   r           Request structure.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void free_request(Request *r) {
    if (!r) {
    	return;
    }

    /* Close socket or fd */
    if(r->file != NULL)
    {
        fclose(r->file);
    }
    else if(r->fd > 0)
    {
        close(r->fd);
    }

    /* Free allocated strings */
    if(r->host != NULL)
    {
        free(r->host);
    }

    if(r->port != NULL)
    {
        free(r->port);
    }

    /* Free headers */
    for(Header * head = r->headers; head != NULL; )
    {
        Header * curr = head;
        head = head->next;
        free(curr);
    }

    /* Free request */
    free(r);
}

/**
 * Parse HTTP Request.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int parse_request(Request *r) {
    /* Parse HTTP Request Method */
    if(parse_request_method(r) < 0)
    {
        return -1;
    }

    /* Parse HTTP Request Headers*/
    if(parse_request_headers(r) < 0)
    {
        return -1;
    }

    return 0;
}

/**
 * Parse HTTP Request Method and URI.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int parse_request_method(Request *r) {
    char buffer[BUFSIZ];
    char *method;
    char *uri;
    char *query;

    /* Read line from socket */
    if((fgets(buffer, BUFSIZ, r->file)) == NULL || strlen(buffer) == 0)
    {
        log("No more requests to read: \n");
        goto fail;
    }

    /* Parse method and uri */
    method = strtok(buffer, WHITESPACE);
    if(method == NULL || strlen(method) == 0)
    {
        log("Cannot find method\n");
        goto fail;
    }

    uri = strtok(NULL, " ");
    if(uri == NULL || strlen(uri) == 0 || uri[0] != '/')
    {
        log("Cannot find uri\n");
        goto fail;
    }

    /* Parse query from uri */
    uri = strtok(uri, "?");
    query = strtok(NULL, " ");

    /* Record method, uri, and query in request struct */
    if((r->method = calloc(1, strlen(method) + 1)) == NULL)
    {
        log("Unable to allocate memory: %s\n", strerror(errno));
        goto fail;
    }
    else
    {
        strcpy(r->method, method);
    }

    if((r->uri = calloc(1, strlen(uri) + 1)) == NULL)
    {
        log("Unable to allocate memory: %s\n", strerror(errno));
        goto fail;
    }
    else
    {
        strcpy(r->uri, uri);
    }

    if(query != NULL || (r->query = calloc(1, strlen(query) + 1)) == NULL)
    {
        log("Unable to allocate memory: %s\n", strerror(errno));
        goto fail;
    }
    else
    {
        strcpy(r->query, query);
    }
    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

fail:
    return -1;
}

/**
 * Parse HTTP Request Headers.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int parse_request_headers(Request *r) {
    struct header *curr = NULL;
    char buffer[BUFSIZ];
    char *name;
    char *value;

    /* Parse headers from socket */
    while(fgets(buffer, BUFSIZ, r->file) != NULL && strlen(buffer) > 0)
    {
        name = strtok(buffer, ":");
        if(name == NULL || strlen(name) == 0)
        {
            log("Couldn't find name\n");
            goto fail;
        }

        // skip space
        strtok(NULL, " ");
        value = strtok(NULL, "");
        if(value == NULL || strlen(value) == 0)
        {
            log("Couldn't find value\n");
            goto fail;
        }

        // create and allocate header
        if((curr = calloc(1, sizeof(Header))) == NULL)
        {
            log("Couldn't allocate memory: %s\n", strerror(errno));
            goto fail;
        }

        if((curr->name = calloc(1, sizeof(strlen(name) + 1))) == NULL)
        {
            log("Couldn't allocate memory: %s\n", strerror(errno));
            goto fail;
        }
        else
        {
            strcpy(curr->name, name);
        }

        if((curr->value = calloc(1, sizeof(strlen(value) + 1))) == NULL)
        {
            log("Couldn't allocate memory: %s\n", strerror(errno));
            goto fail;
        }
        else
        {
            strcpy(curr->value, value);
        }
        
        // if list doesn't have root
        if(r->headers == NULL)
        {
            r->headers = curr;
        }
        
        // move to next header
        curr = curr->next;
        
    }

#ifndef NDEBUG
    for (struct header *header = r->headers; header != NULL; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->value);
    }
#endif
    return 0;

fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
