//for shared types and prototypes

#ifndef SHELL_H
#define SHELL_H
#include <stddef.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64

//parsing
int parse_command(char *command, char *argv[], int max_args);

//builtins
int handle_echo(char *command);
int handle_pwd(char *command);
int handle_cd(char *command);
int handle_type(char *command);
int is_builtin(const char *cmd);

//path
int find_in_path(const char *cmd, char *result, size_t size);

//
void run_external(char *argv[]);
char *builtin_generator(const char *text, int state);
char **builtin_completion(const char *text, int start, int end);
char *executable_generator(const char *text, int state);
void run_pipeline(char *left[], char *right[]);
int handle_history(char *command);
#endif