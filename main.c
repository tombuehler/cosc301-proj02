#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

/* Hey Tom-O */

int main(int argc, char **argv) {
    
    
    char buffer[1024];
    int continueloop = 0;

    while(continueloop == 0){
        printf("Type away: ");
        fgets(buffer, 1024, stdin);
    }


    return 0;
}

