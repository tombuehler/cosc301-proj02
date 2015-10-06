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


char** tokenify(const char *s) {
    char* copy = strdup(s);
    const char* whitespace= " \t\n";
    char* token;
    int numtoks = 0;
    for(token = strtok(copy, whitespace);
        token != NULL; 
        token = strtok(NULL, whitespace)){

            numtoks++;
        }
    char* copy2 = strdup(s);
    char** tokens = malloc(sizeof(char*)*(numtoks+1));
    int current_tok = 0;
    for(token = strtok(copy2, whitespace);
        token != NULL; 
        token = strtok(NULL, whitespace)){

            tokens[current_tok] = strdup(token);
            current_tok++;
        }
    tokens[current_tok] = NULL;
    free(copy);
    free(copy2);
    return tokens;
}

int main(int argc, char **argv) {
    
    char buffer[1024];
    int continueloop = 0;
    char** tokens;
    char* commentStart;

    while(continueloop == 0){
        printf("Type away: ");
        fgets(buffer, 1024, stdin);
        commentStart = strchr(buffer, '#');   //remove all commented characters by ending string at the first #
        if (commentStart != NULL){
            *commentStart = '\0';
        }
        tokens = tokenify(buffer);

    }

    return 0;
}

