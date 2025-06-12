#include <stdio.h>      
#include <stdlib.h>      
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>     
#include <string.h>
#include <sys/types.h>  
#include <sys/wait.h>    
#include <errno.h> 

void RedirectToFile(char *commandOne, char *commandTwo);
void RedirectFromFile(char *commandOne, char *commandTwo);
void pipeOutput(char *commandOne, char *commandTwo, char *commandThree, char nextSymbol);
