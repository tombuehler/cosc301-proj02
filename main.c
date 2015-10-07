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

//Splits 's' up at each occurrence of 'delim.' Returns array of pieces.  
char** tokenify(const char *s, const char* delim) {
    char* copy = strdup(s);
    char* token;
    int numtoks = 0;
    for(token = strtok(copy, delim);
        token != NULL; 
        token = strtok(NULL, delim)){

            numtoks++;
        }
    char* copy2 = strdup(s);
    char** tokens = malloc(sizeof(char*)*(numtoks+1));
    int current_tok = 0;
    for(token = strtok(copy2, delim);
        token != NULL; 
        token = strtok(NULL, delim)){

            tokens[current_tok] = strdup(token);
            current_tok++;
        }
    tokens[current_tok] = NULL;
    free(copy);
    free(copy2);
    return tokens;
}

//Replaces the first # with a null term char
void removeComments(char* input){
    char* commentStart;
    commentStart = strchr(input, '#');
    if (commentStart != NULL){
        *commentStart = '\0';
    }
}

int arrLen(char** arr){
    int i;
    for(i = 0; arr[i] != NULL; i++){ } 
    return i;
}

int main(int argc, char **argv) {
    const char* whitespace= " \t\n";
    char buffer[1024];
    int continueloop = 0;
    char** commands;

    while(continueloop == 0){
        printf("Type away: ");
        fgets(buffer, 1024, stdin);
        removeComments(buffer);
        commands = tokenify(buffer, ";");
        for (int i = 0; i < arrLen(commands); i++){
            printf("Command %d: %s\n", i, commands[i]);
        }
    }
    return 0;
}

