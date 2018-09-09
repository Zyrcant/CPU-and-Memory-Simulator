#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char **argv)
{
  pid_t pid;
  pid=fork();
  if(pid == -1)
  {
    exit(-1);
  }
  if(pid == 0)
  {
    printf("This is the child!\n");
    exit(0);
  }
  waitpid(-1, NULL, 0);
  printf("Parent\n");
  return(0);
}
