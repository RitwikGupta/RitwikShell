#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define COMMANDSIZE 4096
#define MAXTOKENS 25
#define DELIMITERS " \t\n()<>|&;"

static int kill_signal = 1;

void control_c_handler(int x) {
  kill_signal = 0;
  printf("\nExiting Ritwik's shell!\n");
  exit(EXIT_SUCCESS); //Looked up the Ctrl+C exit code online
}

int main(int argc, char **argv) {
  int token_counter, param_list_size, fork_status, out_redir_count, in_redir_flag, i;
  char *tok, *file_string;
  char *command[1], *param_list[MAXTOKENS + 1], *empty_string[2];
  char input[COMMANDSIZE] = {0};
  pid_t parent, child;
  FILE *fp;

  // Handle Ctrl+C
  signal(SIGINT, control_c_handler);

  empty_string[0] = "";
  out_redir_count = 0;

  while(kill_signal) {
    while(strlen(input) == 0) {
      printf("> ");
      fgets(input, COMMANDSIZE, stdin);

      // Removing trailing whitespace
      if(input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = '\0';
      }
    }

    // Count output redirection tokens
    for (i = 0; i < strlen(input); i++) {
      if(input[i] == '>') {
        out_redir_count++;
      }
    }

    strchr(input, '<') ? (in_redir_flag = 1) : (in_redir_flag = 0);

    if(out_redir_count > 0) {
      file_string = strrchr(input, '>') + 2;
      for(i = strrchr(input, '>') - input + 1; i < strlen(input); i++) {
        input[i] = '\0';
      }
    } else if(in_redir_flag) {
      for(i = strrchr(input, '>') - input + 1; i < strlen(input); i++) {
        input[i] = '\0';
      }
      file_string = strrchr(input, '>') + 2;
    }

    token_counter = 0;
    tok = strtok(input, DELIMITERS);
    while(token_counter != MAXTOKENS) {
      if(token_counter == 0) {
        command[0] = tok;
      }
      param_list[token_counter++] = tok;
      tok = strtok(NULL, DELIMITERS);
    }

    // If there are parameters, param_list_size = 1
    param_list[0] == NULL ? (param_list_size = 0) : (param_list_size = 1);

    parent = getpid();
    if(strcmp(command[0], "cd") == 0) {
      if(chdir(param_list[1]) != 0) {
        // Handle error
      }
    } else {
      child = fork();
    }

    if(strcmp(command[0], "exit") == 0) {
      return 0;
    }

    if (child > 0) {
      waitpid(child, &fork_status, 0);
      out_redir_count = 0;
    } else {
      if(param_list_size) {
        if(out_redir_count == 1) { // If the output of the command should be written to a file
          fp = freopen(file_string, "w+", stdout);
          if(execvp(command[0], param_list) == -1) {
            perror("EXECVP: ");
          }
          fclose(fp);
        } else if(out_redir_count == 2) { // If the output of the command should be appended to a file
          fp = freopen(file_string, "a", stdout);
          if(execvp(command[0], param_list) == -1) {
            perror("EXECVP: ");
          }
          fclose(fp);
        } else if(in_redir_flag) { // If a file needs to be passed into stdin
          fp = freopen(file_string, "w", stdin);
          if(execvp(command[0], param_list) == -1) {
            perror("EXECVP: ");
          }
          fclose(fp);
        } else { // If a command just needs to be executed and has flags
          if(execvp(command[0], param_list) == -1) {
            perror("EXECVP: ");
          }
        }
      } else { // If a command needs to be executed without any flags
        if(execvp(command[0], empty_string) == -1) {
          perror("EXECVP: ");
        }
      }
      return -1; // Stay in the shell
    }
    memset(input, '\0', COMMANDSIZE); // Clear the input buffer

  }

  return EXIT_SUCCESS;
}
