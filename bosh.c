/* 

   bosh.c : BOSC shell 

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "/usr/include/readline/readline.h"
#include "/usr/include/readline/history.h"
#include "parser.h"
#include "print.h"
#include "signal.h"

#include <fcntl.h> 

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char **hostname)
{
  FILE *file;

  char cpumodel[HOSTNAMEMAX];
  static char line[HOSTNAMEMAX];
  char *string = line;

  file = fopen ("/proc/sys/kernel/hostname","r");

  while (fgets(line,HOSTNAMEMAX,file))
  {
    //Remove the newline char in the end
    int len = strlen(string);
    if (string[len-1] == '\n') {
      string[len-1] = 0;
    }
    *hostname = string;
  }

  fclose(file);

  return string;
}

/* run a command */
int runcommand(Cmd *cmdlist)
{
  char **cmd = cmdlist->cmd;
  cmdlist = cmdlist->next;

  pid_t pid = -1;
  int fd[2];

  //if there are more commands, we need a pipe 
  if (cmdlist)
  {
    pipe(fd);
    pid = fork();
  }

  if (pid == 0) {
    
    //write
    dup2(fd[1],1);
    close(fd[1]);
    close(fd[0]);

    runcommand(cmdlist);

  }else{
    
    if (cmdlist)
    {
      //read
      dup2(fd[0],0);
      close(fd[0]);
      close(fd[1]);
    }

    execvp(*cmd,cmd); 
    printf("Command not found\n"); 
    exit(0);
  }

  return 0;
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd)
{
  printshellcmd(shellcmd);

  Cmd *cmdlist = shellcmd->the_cmds;

  char **cmd = cmdlist->cmd;
  if (!strcmp(*cmd,"exit"))
  {
    return 1;
  }

  pid_t pid = fork();
    
  if (pid == 0) {

    if (shellcmd->rd_stdout) {
      int fid = open(shellcmd->rd_stdout, O_RDWR|O_CREAT, 0666);
      close(1);
      dup(fid);
    }

    if (shellcmd->rd_stdin) {
      int fid = open(shellcmd->rd_stdin, O_RDONLY);
      close(0);
      dup(fid);
    }

    runcommand(cmdlist);

  }else{
    if (!shellcmd->background) {
      waitpid(pid,NULL,0);
    }
  }

  return 0;
}

/* ctrl-c signal handler */
void INThandler(int sig)
{
  /* Nothing */
}

/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* set the ctrl-c signal */
  signal(SIGINT,INThandler);

  /* initialize the shell */
  char *cmdline;
  char *hostname = NULL;

  int terminate = 0;
  Shellcmd shellcmd;

  if (gethostname(&hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
        if(*cmdline) {
          add_history(cmdline);
          if (parsecommand(cmdline, &shellcmd)) {
            terminate = executeshellcmd(&shellcmd);
          }
        }
        free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }    

  return EXIT_SUCCESS;
}
