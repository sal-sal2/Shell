#include <stdio.h>
#include <string.h>
#include "shell.h"


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
        
        //handle pwd command
        if (handle_pwd(command)){
          continue;
        }

        //handle type command
        if (handle_type(command)){
        continue;
        }

        // handle cd command
        if (handle_cd(command)) {
          continue;
        }

        char *argv_exec[64];
        char command_copy[BUFFER_SIZE];

        strcpy(command_copy, command);
        parse_command(command_copy, argv_exec, 64);//split the command into argv_exec

        //only run if there is a program name
        if (argv_exec[0] != NULL) {
            run_external(argv_exec);
        }

    }
    return 0;
}