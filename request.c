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
        fatal("Couldn't allocate memory: %s", strerror(errno));
        goto fail;
    }

    /* Accept a client */
    r->fd = accept(sfd, &raddr, &rlen);
    if(r->fd < 0)
    {
        fatal("accept failed: %s", strerror(errno));
        goto fail;
    }

    /* Lookup client information */
    if((getnameinfo(&raddr, rlen, r->host, NI_MAXHOST, r->port, NI_MAXHOST, 0)) < 0)
    {
        fatal("Failed to get client info: %s", strerror(errno));
        goto fail;
    }

    /* Open socket stream */
    r->file = fdopen(r->fd, "w+");
    if(!r->file)
    {
        fatal("Couldn't open stream: %s", strerror(errno));
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
    if(strlen(r->host) > 0)
    {
        free(r->host);
    }

    if(strlen(r->port) > 0)
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
    debug("Parse HTTP request method");
    if(parse_request_method(r) < 0)
    {
        return -1;
    }

    /* Parse HTTP Request Headers*/
    debug("Parse HTTP request headers");
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
    debug("Reading line from socket");
    if((fgets(buffer, BUFSIZ, r->file)) == NULL || strlen(buffer) == 0)
    {
        fatal("No more requests to read");
        goto fail;
    }

    chomp(buffer);

    debug("Buffer: %s", buffer);

    /* Parse method and uri */
    debug("Parsing method and uri");
    method = strtok(buffer, " ");
    if(method == NULL || strlen(method) == 0)
    {
        fatal("Cannot find method");
        goto fail;
    }

    log("Method: %s", method);

    uri = strtok(NULL, " ");
    if(uri == NULL || strlen(uri) == 0 || uri[0] != '/')
    {
        fatal("Cannot find uri");
        goto fail;
    }

    log("Uri: %s", uri);

    /* Parse query from uri */
    debug("Parsing query from uri");
    uri = strtok(uri, "?");
    query = strtok(NULL, " ");

    log("Uri: %s", uri);
    log("Query: %s", query);

    /* Record method, uri, and query in request struct */
    debug("Recording method");
    r->method = strdup(method);
    log("Method: %s", r->method);

    debug("Recording uri");
    r->uri = strdup(uri);
    log("Uri: %s", r->uri);

    debug("Recording query");
    if(query) r->query = strdup(query);
    else r->query = NULL;

    log("Query: %s", r->query);

    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    debug("Finished processing method");
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
    char *name = buffer;
    char *value = NULL;

    /* Parse headers from socket */
    debug("Parsing headers from socket");
    while(fgets(buffer, BUFSIZ, r->file) != NULL && strlen(buffer) > 0)
    {
        chomp(buffer);
        log("Header buffer: %s", buffer);

        // get value
        value = skip_nonwhitespace(name);
        value = skip_whitespace(value);
        if(value == NULL || strlen(value) == 0)
        {
            fatal("Couldn't find value");
            goto fail;
        }

        log("Value: %s", value);

        // get name
        debug("Get name");
        name = strtok(buffer, ":");
        if(name == NULL || strlen(name) == 0)
        {
            fatal("Couldn't find name");
            goto fail;
        }

        log("Name: %s", name);

        // skip space
        debug("Skip space");
        

        // create and allocate header
        debug("Allocate header");
        if((curr = calloc(1, sizeof(Header))) == NULL)
        {
            fatal("Couldn't allocate memory: %s", strerror(errno));
            goto fail;
        }

        debug("Allocate name");
        curr->name = strdup(name);

        debug("Allocate value");
        curr->value = strdup(value);
        
        // if list doesn't have root
        debug("Set first header");
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
