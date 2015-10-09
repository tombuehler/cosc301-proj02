#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
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

//used for output of tokenify
int arrLen(char** arr){
    int i;
    for (i = 0; arr[i] != NULL; i++) { }
    return i;
}

//used to free output of tokenify
void freeTokens(char** tokens){
    int i;
    for (i = 0; i < arrLen(tokens); i++){
        free(tokens[i]);
    }
    free(tokens[i]);
    free(tokens);
}

//check for built-in commands
bool built_in(char** arguments, int* state){
    if (strcmp(arguments[0], "mode") == 0){     
        if (strcmp(arguments[1], "p") == 0){
            *state = 1;
            return true;
        }
        else if (strcmp(arguments[1], "s") == 0){
            *state = 0;
            return true;
        }
    }
    else if (strcmp(arguments[0], "parallel") == 0){
        *state = 1;
        return true;
    }
    else if (strcmp(arguments[0], "sequential") == 0) {
        *state = 0;
        return true;
    }
    else if (strcmp(arguments[0], "exit") == 0) {
        *state = 2;
        return true;
    }
    return false;
}

//Parses commands sequentially
int seqParse(char*** commands){
    const char* whitespace = " \t\n";
    char** arguments;
    bool is_built_in = true;
    int state = 0;
    int* status = 0;
    pid_t pid;
    for (int i = 0; (*commands)[i] != NULL; i++){
        arguments = tokenify((*commands)[i], whitespace);
        is_built_in = built_in(arguments, &state);
        if (!is_built_in){
            pid = fork();
            if (pid == 0){   //child process
                execv(arguments[0], arguments);//execv the command
                freeTokens(arguments);
                printf("Command not valid\n");      //if execv returns, command was invalid
                exit(0);
            }
            else{   //parent process
                waitpid(pid, status, 0);    // wait for child process to end
            }
        }
        freeTokens(arguments);
    }
    return state;
}

//Parse in parallel
int parParse(char*** commands, pid_t* waitArr[]){
    const char* whitespace = " \t\n";
    char** arguments;
    bool is_built_in = true;
    int state = 1;
    pid_t pid;
    int i;

    for (i = 0; (*commands)[i] != NULL; i++){
        arguments = tokenify((*commands)[i], whitespace);
        is_built_in = built_in(arguments, &state);  //check if built in
        if (!is_built_in){
            pid = fork();       //create child process
            if (pid == 0){
                execv(arguments[0], arguments);
                printf("Command not valid\n");
                exit(0);
            }
            else{
                (*waitArr)[i] = pid;
            }
        }
        free(arguments);
    }
    waitArr[i] = 0; 
    return state;
}

//sees if a string is empty
bool is_empty(char* input){
    char* temp = strdup(input);
    char** tokens = tokenify(temp, " \n\t");
    if (tokens[0] == NULL){
        free(temp);
        freeTokens(tokens);
        return true;
    }
    free(temp);
    freeTokens(tokens);
    return false;
}

int main(int argc, char **argv) {
    char buffer[1024];
    int continueloop = 0;
    char** commands;
    int state = 0; // 0 = sequential, 1 = parallel, 2 = exit
    pid_t* arr = malloc(50 * sizeof(pid_t));
    arr[0] = 0;
    int status = 0;

    while(continueloop == 0){
        if (state == 0) printf("\nOperating in sequential mode\n");
        else if (state == 1) printf("\nOperating in parallel mode\n");
        printf("Type away>> ");
        fgets(buffer, 1024, stdin);
        if (is_empty(buffer)){
            strcpy(buffer, "mode s");
        }
        removeComments(buffer);
        commands = tokenify(buffer, ";");
        if (state == 0){
            state = seqParse(&commands);
        }
        else if (state == 1){
            state = parParse(&commands, &arr);
            for (int i = 0; arr[i] != 0; i++){    //wait for all child processes to end before printing prompt
                waitpid(arr[i], &status, 0);
            }
            free(arr);
        }
        if (state == 2){
            printf("Goodbye\n");
            freeTokens(commands);
            free(arr);
            return 0;
        }
        freeTokens(commands);
    }
    freeTokens(commands);
    free(arr);
    return 0;
}

