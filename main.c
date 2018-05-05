#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    Request * r = NULL;
    
    int sfd = socket_listen("9962");
    printf("%d\n", sfd);

    accept_request(sfd);
    free_request(r);

    close(sfd);

}
