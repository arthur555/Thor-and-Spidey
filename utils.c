/* utils.c: spidey utilities */
#include "spidey.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>


/**
 * Determine mime-type from file extension.
 *
 * @param   path        Path to file.
 * @return  An allocated string containing the mime-type of the specified file.
 *
 * This function first finds the file's extension and then scans the contents
 * of the MimeTypesPath file to determine which mimetype the file has.
 *
 * The MimeTypesPath file (typically /etc/mime.types) consists of rules in the
 * following format:
 *
 *  <MIMETYPE>      <EXT1> <EXT2> ...
 *
 * This function simply checks the file extension version each extension for
 * each mimetype and returns the mimetype on the first match.
 *
 * If no extension exists or no matching mimetype is found, then return
 * DefaultMimeType.
 *
 * This function returns an allocated string that must be free'd.
 **/
char * determine_mimetype(const char *path) {
    char *ext;
    char *mimetype;
    char *token;
    char buffer[BUFSIZ];
    FILE *fs = NULL;
    
    ext = strstr(path, ".");
    ext++;
    fs = fopen(MimeTypesPath,"r");
    if(!fs)
    {
        debug("Failed to open MimeTypePath: %s", MimeTypesPath);
    }
    while(fgets(buffer, BUFSIZ, fs)){
        mimetype = strtok(buffer," \t\r\n\v");
        token = mimetype;
       // printf("Mimetype1: %s\n", mimetype);
        while(token!= NULL){
            token = strtok(NULL, " \r\n");
            token = skip_whitespace(token);
         //   printf("Mimetype2:hh %s..", token);

            if (token == NULL) continue;
            if (streq(token, ext))
                {
                    debug("Mimetype matched: %s", mimetype);
                    
                    char * mime = calloc(1, strlen(mimetype)+1);
                    strcpy(mime, mimetype);
                    return mime;
                }

        }
    }
    fclose(fs);
    debug ("Mimetype not found, using defualt mimetype: %s", DefaultMimeType);
    char * mime = calloc(1, strlen(DefaultMimeType)+1);
    strcpy(mime, DefaultMimeType);
    /* Find file extension */
    
    /* Open MimeTypesPath file */

    /* Scan file for matching file extensions */

    return NULL;
}

/**
 * Determine actual filesystem path based on RootPath and URI.
 *
 * @param   uri         Resource path of URI.
 * @return  An allocated string containing the full path of the resource on the
 * local filesystem.
 *
 * This function uses realpath(3) to generate the realpath of the
 * file requested in the URI.
 *
 * As a security check, if the real path does not begin with the RootPath, then
 * return NULL.
 *
 * Otherwise, return a newly allocated string containing the real path.  This
 * string must later be free'd.
 **/
char * determine_request_path(const char *uri) {
    
//    char pa[BUFSIZ];
//    strcat(pa, RootPath);
//    strcat(pa, uri);
    char* real_path = realpath(uri, NULL);
    if (real_path==NULL)
    {
        debug("real_path not found");
        return NULL;
    }
    
    char* check_path = realpath(RootPath, NULL);
    if (check_path==NULL)
    {
        debug("check_path not found");
        free(real_path);
        return NULL;
    }
    
    printf("%s\n", real_path);
    printf("%s\n", check_path);
    
    if (strstr(real_path, check_path) != real_path)
        return NULL;

    free(check_path);

    return real_path;
}

/**
 * Return static string corresponding to HTTP Status code.
 *
 * @param   status      HTTP Status.
 * @return  Corresponding HTTP Status string (or NULL if not present).
 *
 * http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 **/
const char * http_status_string(HTTPStatus status) {
    static char *StatusStrings[] = {
        "200 OK",
        "400 Bad Request",
        "404 Not Found",
        "500 Internal Server Error",
        "418 I'm A Teapot",
    };
    if(status < 5)
        return StatusStrings[status];
    return NULL;
}

/**
 * Advance string pointer pass all nonwhitespace characters
 *
 * @param   s           String.
 * @return  Point to first whitespace character in s.
 **/
char * skip_nonwhitespace(char *s) {
    while(s != NULL && strchr(WHITESPACE, *s))
        s++;
    return s;
}

/**
 * Advance string pointer pass all whitespace characters
 *
 * @param   s           String.
 * @return  Point to first non-whitespace character in s.
 **/
char * skip_whitespace(char *s) {
    while( s !=NULL && ((*s)==' '|| (*s)=='\t'))
        s++;
    return s;
}

/*int main(int argc, char* argv[])
{
    char* a = "script.cgi";
    char *z = determine_request_path(a) ;
    if (z)
        printf("%s",z);
    else 
        printf ("URI not found");
    return 0;
	
}*/
/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
