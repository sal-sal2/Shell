#include "completion/completion.h"
#include "builtins/builtin.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <readline/readline.h>

#define MAX_MATCHES 1024

static char *command_generator(const char *text, int state);
static void free_matches(void);

static char *matches[MAX_MATCHES];
static int match_count = 0;

char **builtin_completion(const char *text, int start, int end) {
    (void)end;

    if (start != 0){
        return NULL;
    }
    return rl_completion_matches(text, command_generator);
}

static void free_matches(void) {
    for (int i = 0; i < match_count; i++)
    {
        free(matches[i]);
        matches[i] = NULL;
    }
    match_count = 0;
}

static void build_match_list(const char *text) {
    // Free previous matches
    free_matches();
    
    int text_len = strlen(text);

    // Builtin
    const char **builtins = builtin_get_names();
    for (int i = 0; builtins && builtins[i]; i++)
    {
        if (strncmp(builtins[i], text, text_len) == 0)
        {
            matches[match_count++] = strdup(builtins[i]);
        }
    }

    // Path executable
    char *path_env = getenv("PATH");
    if (!path_env)
        return;

    char *path_copy = strdup(path_env);
    if (!path_copy)
        return;

    char *dir_path = strtok(path_copy, ":");

    while (dir_path && match_count < MAX_MATCHES - 1) {
        DIR *dir = opendir(dir_path);
        if (dir)
        {
            struct dirent *entry;

            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_name[0] == '.')
                    continue;

                if (strncmp(entry->d_name, text, text_len) == 0)
                {
                    char full_path[BUFFER_SIZE];
                    snprintf(full_path, sizeof(full_path),
                             "%s/%s", dir_path, entry->d_name);

                    if (access(full_path, X_OK) == 0)
                    {
                        matches[match_count++] = strdup(entry->d_name);
                        if (match_count >= MAX_MATCHES - 1)
                            break;
                    }
                }
            }

            closedir(dir);
        }

        dir_path = strtok(NULL, ":");
    }

    free(path_copy);

    matches[match_count] = NULL;
}

static char *command_generator(const char *text, int state) {
    static int index;

    if (state == 0)
    {
        index = 0;
        build_match_list(text);
    }

    if (index < match_count)
        return strdup(matches[index++]);

    return NULL;
}
