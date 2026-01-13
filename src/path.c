#include "shell.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


const char *builtins[] = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd",
  NULL
};



int find_in_path(const char *cmd, char *result, size_t size){
  /*
  (const char *, char *, size_t) -> int
  Function searches for an executable file named cmd in the directories listed in the PATH environment variable. 
  If found, writes the full path into result and returns 1. Returns 0 if the executable is not found.
  */
  //get path
  char *path = getenv("PATH");
  if (path == NULL){
    return 0;
  }

  //get copy to not modify existing string
  char path_copy[BUFFER_SIZE];
  strncpy(path_copy, path, sizeof(path_copy));
  path_copy[sizeof(path_copy) - 1] = '\0';

  char *dir = strtok(path_copy, ":");

  while (dir != NULL) {
    /*
    for each directory in PATH:
    build "directory/command"
    if I can run it:
        stop and return it
    */
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

    //check if file exists and is executable, stop as soon as we find a valid directory 
    if (access(full_path, X_OK) == 0) {
      strncpy(result, full_path, size);
      result[size - 1] = '\0';
      return 1;
    }

    //get next directory in PATh
    dir = strtok(NULL, ":");
  }
  return 0;
}
int is_builtin(const char *cmd){
  /*
  (const char *) -> int
  Function determines whether the given command name matches one of the shell’s built-in commands. Returns 1 if it is a builtin, otherwise returns 0.
  */
  for (int i = 0; builtins[i] != NULL; i++){
    if (strcmp(cmd, builtins[i]) == 0){
      return 1;
    }
  }
  return 0;
}



