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

//Parses commands sequentially
int seqParse(char*** commands){
    const char* whitespace = " \t\n";
    char** arguments;
    int state = 0;
    int* status = 0;
    pid_t pid;
    for (int i = 0; (*commands)[i] != NULL; i++){
        arguments = tokenify((*commands)[i], whitespace);
        if (strcmp(arguments[0], "mode") == 0){         //check for built-in commands
            if (strcmp(arguments[1], "p") == 0) state = 1;
        }
        else if (strcmp(arguments[0], "parallel") == 0)  state = 1;
        else if (strcmp(arguments[0], "exit") == 0) state = 2;
        else{
            pid = fork();
            if (pid == 0){   //child process
                execv(arguments[0], arguments);     //execv the command
                printf("Command not valid\n");      //if execv returns, command was invalid
            exit(0);
            }
            else{   //parent process
                waitpid(pid, status, 0);    // wait for child process to end
            }
            free(arguments);
        }
    }
    return state;
}

int main(int argc, char **argv) {
    char buffer[1024];
    int continueloop = 0;
    char** commands;
    int state = 0; // 0 = sequential, 1 = parallel, 2 = exit

    while(continueloop == 0){
        if (state == 0) printf("\nOperating in sequential mode\n");
        else if (state == 1) printf("\nOperating in parallel mode\n");
        printf("Type away>> ");
        fgets(buffer, 1024, stdin);
        removeComments(buffer);
        commands = tokenify(buffer, ";");
        if (state == 0){
            state = seqParse(&commands);
        }
        else if (state == 1){
            //state = parParse(&commands);
        }
        if (state == 2){
            printf("Goodbye");
            return 0;
        }
    }
    return 0;
}

