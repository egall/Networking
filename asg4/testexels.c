#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
    FILE* fpipe;
    char* command = "ls -l";
    char line [256];
    char buffer[2048];

    bzero(buffer, sizeof(buffer));
    if( !(fpipe = (FILE*) popen(command, "r"))){
        perror("Problems with pipe\n");
        exit(1);
    } 
    while(fgets(line, sizeof(line), fpipe)){
        strcat(buffer, line);
    }
    printf("\n>ls -l:\n %s\n", buffer);

    pclose(fpipe);
    return 0;
}
