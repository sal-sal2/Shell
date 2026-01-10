#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h> 


#define BUFFER_SIZE 1024

const char *builtins[] = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd",
  NULL
};

int is_builtin(const char *cmd);
int handle_type(char *command);
int find_in_path(const char *cmd, char *result, size_t size);
int parse_command(char *command, char *argv[], int max_args);
void run_external(char *argv[]);
int handle_pwd(char *command);
int handle_cd(char *command);


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


int handle_pwd(char *command){
  /*
  (char *) -> int
  Function checks whether the given command is a pwd command. If so, prints the
  current working directory to standard output and returns 1.
  If the command is not a pwd command or an error occurs, returns 0.
  */
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
  /*
  (char *) -> int
  Function checks whether the given command is a cd command. If so, attempts to
  change the current working directory to the specified path and returns 1.
  Supports changing to the home directory using "~".
  If the command is not a cd command, returns 0.
  */
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

int parse_command(char *command, char *argv[], int max_args){
  /*
  (char *, char *, int) -> int
  Function splits the command on spaces while respecting single/double-quoted strings.
  Quotes are removed, spaces inside quotes are preserved.
  Returns the number of arguments parsed.
  */

  int argc = 0;
  int in_single_quotes = 0;
  int in_double_quotes = 0;
  char *p = command;
  char *arg_start = NULL;

  while (*p){
    //skip leading spaces
    while(*p == ' ' && !in_single_quotes && !in_double_quotes){
      p++;
    }

    if (*p == '\0'){
      break;
    }

    //mark the start of the argument
    arg_start = p;
    argv[argc++] = p; //store pointer to start of argument

    //go through characters of current argument
    while (*p){

      if (*p == '\'' && !in_double_quotes) {
        in_single_quotes = !in_single_quotes;

        //remove quote by shifting
        memmove(p, p + 1, strlen(p));
        continue;
      }

      if (*p == '"' && !in_single_quotes) {
          in_double_quotes = !in_double_quotes;

          //remove quote by shifting
          memmove(p, p + 1, strlen(p));
          continue;
      }

      //if space outside of quotes, the argument ends
      if (*p == ' ' && !in_single_quotes && !in_double_quotes) {
        //Null-terminate the current argument and move past the space
        *p = '\0';        
        p++;            
        break;
      }

      //handle backslashes
      if (*p == '\\') {
        if (in_double_quotes) {
          //only \ and " are special
          char next = *(p + 1);
          if (next == '"' || next == '\\') {
              //shift left to remove backslash
              memmove(p, p + 1, strlen(p));
              
              //move past the escaped character
              p++;
              continue;
          }
          //leave the backslash as literal
          p++;
          continue;
      } 
      else if (!in_single_quotes) {
          //outside quotes, backslash escapes any char
          memmove(p, p + 1, strlen(p));
          if (*p == '\0'){
            break;
          }

          p++;
          continue;
        }
      }

      //move to the next character
      p++;
    }

      //stop when exceeding maximum number of arguments
      if (argc >= max_args - 1){
        break;
      }
    }

    argv[argc] = NULL;
    return argc;
}

void run_external(char *argv[]){
  /*
  Function attempts to execute an external program specified in argv[0] with the arguments in argv[].
  Searches PATH for the executable. Forks a child process to run the program.
  The parent waits for the child to finish.
  Prints "command not found" if the program does not exist.
  */
  char full_path[BUFFER_SIZE];

  if (!find_in_path(argv[0], full_path, sizeof(full_path))){
    printf("%s: command not found\n", argv[0]);
    return;
  }

  //create a child process
  pid_t pid = fork();

  if (pid == 0){
    //replace child process with program
    execv(full_path, argv);
    perror("execv");
    exit(1);
  }
  else if (pid > 0){
    //parent process, wait for child to end
    wait(NULL);
  } else{
    //failed fork
    perror("fork");
  }
}