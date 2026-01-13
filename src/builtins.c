#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>



int handle_echo(char *command){
  /*
  (char *) -> int
  Function checks whether the given command is an echo command. If so, prints the text following echo to standard output and returns 1. 
  If the command is not an echo command, returns 0.
  */

  if (strncmp(command, "echo", 4) != 0 || (command[4] != ' ' && command[4] != '\0')){
    return 0;
  }

  if (command[4] == ' '){
    printf("%s\n", command + 5);
  } else{
    printf("\n");
  }

  return 1;
}
int handle_pwd(char *command){
  /**/
  if (strncmp(command, "pwd", 3) || (command[3] != ' ' && command[3] != '\0')){
    return 0;
  }

  char buf[BUFFER_SIZE];
  if (getcwd(buf, sizeof(buf)) != NULL){
    printf("%s\n", buf);
    return 1;
  }

  return 0;
}
int handle_cd(char *command){
  if (strncmp(command, "cd", 2) != 0 || (command[2] != ' ' && command[2] != '\0')){
    return 0;
  }

  //no argument
  if (command[2] == '\0'){
    return 1;
  }

  char *path = command + 3;

  if (strcmp(path, "~") == 0) {
    path = getenv("HOME");
    if (!path) {
        printf("cd: HOME not set\n");
        return 1;
    }
  }
  
  struct stat st;

  //check if directory exists
  if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)){
    printf("cd: %s: No such file or directory\n", path);
    return 1;
  }

  //check permissiions and change directory
  if (chdir(path) != 0){
    printf("cd: %s: No such file or directory\n", path);
  }

  return 1;
}
int handle_type(char *command){
  /*
  (char *) -> int
  Functions Checks whether the given command is a type command. If so, determines whether the argument is a shell builtin or an executable found in PATH, prints the result, and returns 1. 
  Returns 0 if the command is not a type command.
  */
  
  if (strncmp(command, "type", 4) != 0 || (command[4] != ' ' && command[4] != '\0')){
    return 0;
  }

  //no argument after type
  if (command[4] == '\0'){
    return 1;
  }

  char *arg = command + 5;

  //builtin check
  if (is_builtin(arg)){
    printf("%s is a shell builtin\n", arg);
    return 1;
  } 

  //path search
  char full_path[BUFFER_SIZE];
  if (find_in_path(arg, full_path, sizeof(full_path))){
    printf("%s is %s\n", arg, full_path);
    return 1;
  }
  
  //not found
  printf("%s: not found\n", arg);
  return 1;
}