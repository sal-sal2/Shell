#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define BUFFER_SIZE 1024



int main(int argc, char *argv[]) {
    //flush after every printf
    setbuf(stdout, NULL);

    char command[BUFFER_SIZE];

    while (1){
        printf("$ ");

        fgets(command, sizeof(command), stdin);

        //remove the newline character
        command[strcspn(command, "\n")] = '\0';

        //handle exit command
        if (strcmp(command, "exit") == 0){
        return 0;
        }
    }


}