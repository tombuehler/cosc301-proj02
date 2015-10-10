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

struct _dir_node{
    char dir[1024];
    struct _dir_node* next;
}; typedef struct _dir_node dir_node;

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

//Replaces first 'term' with null term char
void endStr(char* input, char term){
    char* end;
    end = strchr(input, term);
    if (end != NULL){
        *end = '\0';
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
    if (arguments[0] == NULL) {
        return false;
    }
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

//finds the correct path in shell-config, or does nothing
void find_path(char** arguments, dir_node* dir_list){
    struct stat statresult;
    char* command = arguments[0];
    int rv = stat(command, &statresult);
    if (rv == 0){
        execv(command, arguments);
    }
    dir_node* current = dir_list;
    char root[1024];
    while(current != NULL){
        strcpy(root, current->dir);
        strcat(root, "/");
        strcat(root, command);
        rv = stat(root, &statresult);
        if (rv >= 0){
            execv(root, arguments);
        }
        current = current->next;
    }
}

//Parses commands sequentially
void seqParse(char** commands, int* state, dir_node* dir_list){
    const char* whitespace = " \t\n";
    char** arguments;
    bool is_built_in = true;
    pid_t pid;

    for (int i = 0; commands[i] != NULL; i++){
        arguments = tokenify(commands[i], whitespace);
        is_built_in = built_in(arguments, state);
        if (!is_built_in){
            pid = fork();
            if (pid == 0){   //child process
                find_path(arguments, dir_list);
                freeTokens(arguments);
                printf("Command not valid\n");
                exit(0);
            }
            else{   //parent process
                waitpid(pid, NULL, 0);    // wait for child process to end
            }
        }
        freeTokens(arguments);
    }
}

//waits for every pid in pid arr
void waitParallel(int* pidArr){
    for(int i = 0; pidArr[i] != 0; i++){    
        waitpid(pidArr[i], NULL, 0);
    }
}

//Parse in parallel
void parParse(char** commands, int* state, dir_node* dir_list){
    const char* whitespace = " \t\n";
    char** arguments;
    bool is_built_in = true;
    pid_t pid;
    int i;
    pid_t waitArr[arrLen(commands)];
    memset(waitArr, 0, (arrLen(commands)+1)*sizeof(pid_t));

    for (i = 0; (commands)[i] != NULL; i++){
        arguments = tokenify(commands[i], whitespace);
        is_built_in = built_in(arguments, state);  //check if built in
        if (!is_built_in){
            pid = fork();       //create child process
            if (pid == 0){        
                find_path(arguments, dir_list);
                freeTokens(arguments);
                printf("Command not valid\n");
                exit(0);
            }
            else{
                waitArr[i] = pid;
            }
        }
        freeTokens(arguments);
    }

    waitParallel(waitArr);
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

//initializes the linked list, loading a directory into each node
dir_node* init_list(FILE* dir_file){
    if (dir_file == NULL) return NULL;  //file doesn't exist
    dir_node* head = NULL;
    dir_node* current = head;
    char currDir[1024];
    while (fgets(currDir, 1024, dir_file) != NULL){
        endStr(currDir, '\n');
        if (head == NULL){
            head = malloc(sizeof(dir_node));
            current = head;
        }
        strcpy(current->dir, currDir);
        current->next = malloc(sizeof(dir_node));
        current = current->next;
    }
    current = NULL;
    return head;
}

int main(int argc, char **argv) {
    FILE* dir_file = fopen("shell-config", "r");
    dir_node* dir_list = init_list(dir_file);
    
    char buffer[1024];
    char** commands;
    int state = 0; // 0 = sequential, 1 = parallel, 2 = exit   

    while(true){
        if (state == 0) printf("\nOperating in sequential mode\n");
        else if (state == 1) printf("\nOperating in parallel mode\n");
        printf("Type away>> ");
        fgets(buffer, 1024, stdin);
        endStr(buffer, '#');
        commands = tokenify(buffer, ";");
        if (state == 0){
            seqParse(commands, &state, dir_list);
        }
        else if (state == 1){
            parParse(commands, &state, dir_list); 
        }
        if (state == 2){
            printf("Goodbye\n");
            freeTokens(commands);
            return 0;
        }
       freeTokens(commands);
    }
    return 0;
}

