#include "myShell.h"
int main (int argc, char *argv[]){

    pid_t childpid;  
    int status;
    size_t length = 0;
    char *token;
    char delimiter[4] = "><|";
    bool ampersand = false;

    char *line = malloc(500 * sizeof(char));
    char *lineCopy = malloc(500 * sizeof(char));  //copy of line for use with strtok
    char *fileData = NULL;
    char *symbolFunctions = NULL;
    char **files = NULL;
    char **arguments = NULL;

    for ( ;; ) {

        files = malloc(500 * sizeof(char));
        symbolFunctions = malloc(500 * sizeof(char));
        arguments = malloc(500 * sizeof(char));
        ampersand = false;

        printf("> ");
        getline(&line, &length, stdin);

        if (!strncmp(line, "exit", 4)){
            free(line);
            free(lineCopy);
            free(files);
            exit(-1);
        }

        strcpy(lineCopy, line);

        strtok(lineCopy,"\n");
        length - 1;

        if (lineCopy[strlen(lineCopy) - 1] == '&'){
            strtok(lineCopy,"&");
            ampersand = true;
            length - 1;
        }

        for (int j = 0; j < length; j++){
            if ((lineCopy[j] == '>') || (lineCopy[j] == '<') || (lineCopy[j] == '|')){
                symbolFunctions[strlen(symbolFunctions)] = lineCopy[j];
            }
        }

        token = strtok(lineCopy, delimiter);
        int length = strlen(token);

        int i = 0; //keeps track of number of files for char **files
        //breaks up the user input into char **files and gets rid of trailing whitespaces on filenames
        while (token != NULL){
            length = strlen(token);
            if ((length >= 5) && (token[length - 1] == 't') && (token[length - 2] == 'x') && (token[length - 3] == 't') && (token[length - 4] == '.')){
                char *temp = malloc(sizeof(token));
                int j = 0;
                int n = 0;

                while (token[j] != '\0'){
                    if (token[j] != ' '){
                        temp[n++] = token[j];
                    }
                    j++;
                }
                temp[n] = '\0';
                files[i++] = temp;
            }else {
                files[i++] = token;
            }
            token = strtok(NULL, delimiter);
        }

        childpid = fork();

        FILE *file = NULL;
        if ( childpid >= 0 ){
            if ( childpid == 0 ){
                if (strlen(symbolFunctions) != 0){
                    for (int m = 0; m < strlen(symbolFunctions); m++){

                        if (symbolFunctions[m] == '>'){

                            RedirectToFile(files[m], files[m + 1]);
                            
                        }else if (symbolFunctions[m] == '<'){

                            RedirectFromFile(files[m], files[m + 1]);

                        }else if (symbolFunctions[m] == '|'){
                            
                            pipeOutput(files[m], files[m + 1], files[m + 2], symbolFunctions[m + 1]);

                        }

                    }
                }else {

                    token = strtok(lineCopy, " ");

                    i = 0; //used to keep track of current position in arguments
                    while (token != NULL){
                        arguments[i++] = token;
                        token = strtok(NULL, " ");
                    }

                    status = execvp(arguments[0], arguments);

                    exit(status);
                }

            } else {
                if (!ampersand){
                    waitpid(childpid,&status,0);
                }
            }
        } else {
            perror("fork");
            free(line);
            free(lineCopy);
            free(files);
            exit(-1);
        }
        free(files);
    }

    free(arguments);
    free(line);
    free(lineCopy);
    free(fileData);
    free(symbolFunctions);
    free(files);

}


void RedirectToFile(char *commandOne, char *commandTwo){
    char **arguments = malloc(sizeof(char) * 500);

    char *token;

    token = strtok(commandOne, " ");

    int i = 0; //keeps track of number of arguments
    while (token != NULL){
        arguments[i++] = token;
        token = strtok(NULL, " ");
    }

    int fileInt = open(commandTwo, O_CREAT | O_TRUNC | O_WRONLY);

    dup2(fileInt, STDOUT_FILENO);

    int status = execvp(arguments[0], arguments);

    free(arguments);
    free(token);

    exit(status);

}


void RedirectFromFile(char *commandOne, char *commandTwo){
    FILE *file = fopen(commandTwo, "r");

    char *fileData = NULL;
    char **arguments = malloc(sizeof(char) * 500);
    char *token;

    if (file == NULL){
        printf("file not found\n");
        exit(-1);
    }

    strcpy(arguments[0], commandOne);

    fileData = malloc(sizeof(file) + 10);

    char character = fgetc(file);
    int i = 0; //used to keep track of current position in fileData
    while (character != EOF){
        fileData[i++] = character;
        character = fgetc(file);
    }

    token = strtok(fileData, " ");

    i = 1; //used to keep track of current position in arguments
    while (token != NULL){
        arguments[i++] = token;
        token = strtok(NULL, " ");
    }
    
    int status = execvp(arguments[0], arguments);

    free(arguments);
    free(token);
    free(fileData);

    exit(status);
}


void pipeOutput(char *commandOne, char *commandTwo, char *commandThree, char nextSymbol){
    int f[2];
    pipe(f);

    char **arguments = malloc(sizeof(char) * 500);
    char *token;
    int status;

    pid_t childpid1;

    childpid1 = fork();
                            
    if (childpid1 == 0){

        dup2(f[1], STDOUT_FILENO);
        close(f[0]);
        close(f[1]);
        
        token = strtok(commandOne, " ");

        int i = 0; //keeps track of number of arguments
        while (token != NULL){
            arguments[i++] = token;
            token = strtok(NULL, " ");
        }
        
        status = execvp(arguments[0], arguments);

    }

    pid_t childpid2;

    childpid2 = fork();
    
    if (childpid2 == 0){

        dup2(f[0], STDIN_FILENO);
        close(f[0]);
        close(f[1]);

        if (nextSymbol == '>'){
            RedirectToFile(commandTwo, commandThree);
            
        }else {
            token = strtok(commandTwo, " ");

            int i = 0; //keeps track of number of arguments
            while (token != NULL){
                arguments[i++] = token;
                token = strtok(NULL, " ");
            }
            
            status = execvp(arguments[0], arguments);
        }
    }

    close(f[0]);
    close(f[1]);

    waitpid(childpid1,&status,0);
    waitpid(childpid2,&status,0);

    free(arguments);
    free(token);

    exit(status);

}