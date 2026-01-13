#include <string.h>
#include "shell.h"

int parse_command(char *command, char *argv[], int max_args){
  /*
  (char *, char *, int) -> int
  Function splits the command on spaces while respecting single-quoted strings.
  Single quotes are removed, spaces inside quotes are preserved.
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

      //handle backslash outside quotes
      if (*p == '\\' && !in_single_quotes && !in_double_quotes) {
        //remove \ by shifting
        memmove(p, p + 1, strlen(p));  
        if (*p == '\0'){
          break;
        }         
        p++;                           
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
