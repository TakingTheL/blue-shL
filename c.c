#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define DEBUGGING 1 //Set to 1/0 as appropriate

//returns the new fileno of stdin
//changes STDIN_FILENO to target
int redirect_stdin(char* target){
  int fd = open(target, O_CREAT || O_WRONLY);
  int fd_stdin = dup(STDIN_FILENO);  
  dup2( fd, STDIN_FILENO);
  return fd_stdin;
}

//returns the new fileno of stdout
//changes STDOUT_FILENO to target
int redirect_stdout(char* target){
  int fd = open(target, O_CREAT || O_WRONLY);
  int fd_stdin = dup(STDIN_FILENO);  
  dup2( fd, STDIN_FILENO);
  return fd_stdin;
}

//resets the data table
void redirect_reset(int stdin, int stdout){
  dup2( stdin, STDIN_FILENO);
  dup2( stdin, STDOUT_FILENO);
}

char **parse_args(char * line, char *delimiter){
  char **ret = malloc(100);
  ret[0]=line;
  unsigned char i = 1;
  char * args;
  while( args = strsep(&line, delimiter))
    ret[i++]=args;
  ret[i] = 0;
  return ret;
}

void special_funcs(char ** chargs) {
  char *cmd = chargs[0];

  if (DEBUGGING) printf("Special function \"%s\" to be executed\n", cmd);

  if (!strcmp(cmd, "cd"))
    chdir(chargs[2]);
  else if (!strcmp(cmd, "exit"))
    exit(0);    
}

void execute_single(char * line){
  if (!line)
    return;

  // Remove extra chars
  while (*line == ' ' || *line == '\n')
    line++;

  char **chargs = parse_args(line, " ");
  if (!( strcmp(chargs[0],"cd") && strcmp(chargs[0], "exit")))
    return special_funcs(chargs);

  int f = fork(); // Fork off a child to exec commands

  if (!f) {

    if (DEBUGGING) { // Print debug info
      printf("Executing \"%s\" with the following parameters\n", chargs[0]);
      int i = 1;
      while (chargs[i]) {
        printf("%s\n", chargs[i]);
        i++;
      }
      printf("\n");
    }

    execvp(chargs[0], chargs + 1);
  }

  wait(&f);
}

void execute(char * line) {
  if (line)
    *strchr(line, '\n') = 0;
  else
    return;

  char **cmds = parse_args(line, ";");
  unsigned char i = 0;

  if (DEBUGGING) {
    printf("All commands to be run:\n");
    while(cmds[i++])
      printf("CMDS[%d]: %s\n", i, cmds[i]);
    i = 0;
    printf("\n\n");
  }

  while (cmds[i++]){
    if(*cmds[i] == '<')
      redirect_stdin(cmds[i++]);
    else if(*cmds[i] == '>')
      redirect_stdout(cmds[i++]);
    else
      execute_single(cmds[i]);
  }
}

void prompt_user(char * buf) {
  char wd[100];
  printf("\e[36mblue-shL:\e[33m%s\e[37m$ ", getcwd(wd, 100));
  fgets(buf, 100, stdin);
}

int main(){
  char *s = (char *)malloc(101);
  while ( 1 ){
    prompt_user(s);
    execute(s);
  }
  free(s);
  return 0;
}
