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

#include <fcntl.h> 

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
  FILE *file;

  char cpumodel[HOSTNAMEMAX];
  char line[HOSTNAMEMAX];
  file = fopen ("/proc/sys/kernel/hostname","r");

  while (fgets(line,HOSTNAMEMAX,file))
  {
    hostname = "%s";
  }

  fclose(file);

  char temp[HOSTNAMEMAX];
  sprintf(temp, "tester %s",hostname);
  //strcpy(hostname,temp);
  
  return "";
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd)
{
  printshellcmd(shellcmd);
  
  Cmd *cmdlist = shellcmd->the_cmds;

  pid_t pid = fork();
  if (pid == 0) {
    char **cmd = cmdlist->cmd;
    char **printcmd = cmd;
    char * argv[] = {"",NULL};
    
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

    execvp(*printcmd,argv); 
    if (!strcmp(*printcmd,"exit")) {
        printf("Send command to quit parent\n");
        exit(0);
    }
    printf("Command not found\n");
  }else{
    if (!shellcmd->background) {
      waitpid(pid,NULL,0);
    }
  }
  return 0;
}
  
/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];

  strcpy(hostname,"myHostName");
  
  int terminate = 0;
  Shellcmd shellcmd;

  if (gethostname(hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
        if(*cmdline) {
          add_history(cmdline);
          if (parsecommand(cmdline, &shellcmd)) {
            terminate = executeshellcmd(&shellcmd);
            printf("terminate: %d \n",terminate);
          }
        }
        free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }    

  return EXIT_SUCCESS;
}

