#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <fcntl.h>

 
void run_external(char *argv[]){
  /*
  Function attempts to execute an external program specified in argv[0] with the arguments in argv[].
  Searches PATH for the executable. Forks a child process to run the program.
  The parent waits for the child to finish.
  Prints "command not found" if the program does not exist.
  */
  char full_path[BUFFER_SIZE];
  char *out_file = NULL;
  char *err_file = NULL;
  int append_out = 0;
  int append_err = 0;

  //scan argv for redirection
  for (int i = 0; argv[i] != NULL; i++){

    //check >> or 1>>
    if (strcmp(argv[i], ">>") == 0 || strcmp(argv[i], "1>>") == 0) {
      if (argv[i + 1] == NULL) {
        fprintf(stderr, "syntax error: expected file after >>\n");
        return;
      }
      out_file = argv[i + 1];
      append_out = 1;
      argv[i] = NULL;
      break;
    }

    //check > or 1>
    if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "1>") == 0){
      //nothing after
      if(argv[i + 1] == NULL){
        fprintf(stderr, "syntax error: expected file after >\n");
        return;
      }

      //something after
      out_file = argv[i + 1];

      //remove
      argv[i] = NULL;
      break;
    }

    //check for 2>>
    if (strcmp(argv[i], "2>>") == 0) {
      if (argv[i + 1] == NULL) {
        fprintf(stderr, "syntax error: expected file after 2>>\n");
        return;
      }
      err_file = argv[i + 1];
      append_err = 1;
      argv[i] = NULL;
      break;
    }
    //check for 2>
    if (strcmp(argv[i], "2>") == 0) {
      if (argv[i + 1] == NULL) {
        fprintf(stderr, "syntax error: expected file after 2>\n");
        return;
      }
      err_file = argv[i + 1];
      argv[i] = NULL;
      break;
    }
  }

  if (!find_in_path(argv[0], full_path, sizeof(full_path))){
    printf("%s: command not found\n", argv[0]);
    return;
  }

  //create a child process
  pid_t pid = fork();

  if (pid == 0){
    if (out_file) {
    int flags = O_WRONLY | O_CREAT;

    if (append_out) {
      flags |= O_APPEND;
    } else {
      flags |= O_TRUNC;
    }

    int fd = open(out_file, flags, 0644);
    if (fd < 0) {
      perror("open");
      exit(1);
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);
    }

    if (err_file) {
    int flags = O_WRONLY | O_CREAT;

    if (append_err) {
      flags |= O_APPEND;
    } else {
      flags |= O_TRUNC;
    }

    int fd = open(err_file, flags, 0644);
    if (fd < 0) {
      perror("open");
      exit(1);
    }

    dup2(fd, STDERR_FILENO);
    close(fd);
  }
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