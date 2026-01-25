#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>




int main(int argc, char *argv[]) {
  //flush after every printf
  setbuf(stdout, NULL);

  char command[BUFFER_SIZE];

  //start autocomplete
  rl_attempted_completion_function = builtin_completion;

  //add space after completion
  rl_completion_append_character = ' ';

  while (1){
    char *line = readline("$ ");

    if (!line) {
        break;
    }

    if (*line) {
        add_history(line);
    }

    strncpy(command, line, BUFFER_SIZE - 1);
    command[BUFFER_SIZE - 1] = '\0';
    free(line);


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

    //handle history command
    if (handle_history(command)) {
        continue;
    }

    char *argv_exec[64];
    char command_copy[BUFFER_SIZE];

    strcpy(command_copy, command);
    parse_command(command_copy, argv_exec, 64);//split the command into argv_exec

    //look for pipe |
    int pipe_index = -1;
    for (int i = 0; argv_exec[i]; i++) {
        if (strcmp(argv_exec[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }

    if (pipe_index != -1) {
        argv_exec[pipe_index] = NULL;
        char **left = argv_exec;
        char **right = &argv_exec[pipe_index + 1];
        run_pipeline(left, right);
    } else if (argv_exec[0]) {
        run_external(argv_exec);
    }

  }
    return 0;
}

char *builtin_generator(const char *text, int state) {
    static int index;
    static int len;

    const char *builtins[] = { "echo", "exit", NULL };

    if (state == 0) {
        index = 0;
        len = strlen(text);
    }

    while (builtins[index]) {
        const char *name = builtins[index++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}
char **builtin_completion(const char *text, int start, int end) {
    (void)end;

    if (start != 0) {
        return NULL;
    }

    char **matches = NULL;

    matches = rl_completion_matches(text, builtin_generator);
    char **execs = rl_completion_matches(text, executable_generator);

    if (!matches) return execs;
    if (!execs) return matches;

    //merge
    int i, j;
    for (i = 0; matches[i]; i++);
    for (j = 0; execs[j]; j++);

    matches = realloc(matches, (i + j + 1) * sizeof(char *));
    for (int k = 0; k < j; k++) {
        matches[i + k] = execs[k];
    }
    matches[i + j] = NULL;

    free(execs);
    return matches;
}

char *executable_generator(const char *text, int state) {
    static char **paths = NULL;
    static int path_index;
    static DIR *dir;
    static struct dirent *entry;
    static int text_len;

    if (state == 0) {
        text_len = strlen(text);
        path_index = 0;

        char *path_env = getenv("PATH");
        if (!path_env) return NULL;

        static char path_copy[BUFFER_SIZE];
        strncpy(path_copy, path_env, sizeof(path_copy));
        path_copy[sizeof(path_copy) - 1] = '\0';

        static char *path_array[128];
        int i = 0;
        char *token = strtok(path_copy, ":");
        while (token && i < 127) {
            path_array[i++] = token;
            token = strtok(NULL, ":");
        }
        path_array[i] = NULL;
        paths = path_array;
        dir = NULL;
    }

    while (1) {
        if (!dir) {
            if (!paths[path_index]) return NULL;

            dir = opendir(paths[path_index++]);
            if (!dir) {
                continue;
            }
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, text, text_len) == 0) {
                char full_path[BUFFER_SIZE];
                snprintf(full_path, sizeof(full_path),
                         "%s/%s", paths[path_index - 1], entry->d_name);

                if (access(full_path, X_OK) == 0) {
                    return strdup(entry->d_name);
                }
            }
        }

        closedir(dir);
        dir = NULL;
    }
}
void run_pipeline(char *left[], char *right[]) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // First command: stdout → pipe write
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);

        char path[BUFFER_SIZE];
        if (!find_in_path(left[0], path, sizeof(path))) {
            fprintf(stderr, "%s: command not found\n", left[0]);
            exit(1);
        }

        execv(path, left);
        perror("execv");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Second command: stdin ← pipe read
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);

        char path[BUFFER_SIZE];
        if (!find_in_path(right[0], path, sizeof(path))) {
            fprintf(stderr, "%s: command not found\n", right[0]);
            exit(1);
        }

        execv(path, right);
        perror("execv");
        exit(1);
    }

    // Parent
    close(fd[0]);
    close(fd[1]);

    // IMPORTANT: wait for BOTH
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
int handle_history(char *command) {
    // Must start with "history"
    if (strncmp(command, "history", 7) != 0 ||
        (command[7] != '\0' && command[7] != ' ')) {
        return 0;
    }

    int limit = -1;

    // Parse optional number
    if (command[7] == ' ') {
        limit = atoi(command + 8);
        if (limit < 0) {
            limit = 0;
        }
    }

    HIST_ENTRY **list = history_list();
    if (!list) {
        return 1;
    }

    // Count total entries
    int total = 0;
    while (list[total]) {
        total++;
    }

    // Determine starting index
    int start = 0;
    if (limit >= 0 && limit < total) {
        start = total - limit;
    }

    for (int i = start; i < total; i++) {
        printf("%5d  %s\n", i + 1, list[i]->line);
    }

    return 1;
}