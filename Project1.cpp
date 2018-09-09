#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/wait.h>

//helper functions to create instructions
void createInstruction(char* a, char type, int pc);
void createInstruction(char* a, char type, int pc, int location);

int main(int argc, char **argv)
{

  //pipes to communicate between memory and parent
  //pipes is written to by CPU and read by memory. pipes2 is written to by memory and read by CPU
  int pipes[2];
  int pipes2[2];
  pipe(pipes);
  pipe(pipes2);

  //fork process for memory
  pid_t childpid;
  childpid=fork();
  if(childpid == -1)
  {
    perror("Eror creating a child process");
    exit(-1);
  }
  //memory process
  if(childpid == 0)
  {
    //memory
    int mem[2000];
 
    //initialize memory 
    std::ifstream inFile;
    inFile.open(argv[1]); 
    int added;
    std::string rest;
    int position = 0;
    while(inFile >> added) 
    {
      mem[position++] = added;
      std::getline(inFile, rest);
    }

    //testing
    mem[1210] = 12;

    //instruction includes whether to read or write: r is for read, w for write
    char instruc[30];

    //close pipes that aren't being used
    close(pipes[1]);
    close(pipes2[0]);

    //process read/write requests from cpu
    while(true)
    {
    read(pipes[0], instruc, sizeof(instruc));
    std::string str = instruc; 
    std::cout << "The instruction is " << str << std::endl;

    if(instruc[0] == 'r')
    {
      std::string memoryCells = str.substr(2,str.length()-1);
      write(pipes2[1], &mem[std::stoi(memoryCells)], 2);
    }
    else if(instruc[0] == 'w')
    {
      std::string toWrite = str.substr(2, 6);
      std::string location = str.substr(7, str.length()-1);
      std::cout<< "Writing " << toWrite << " to " << location;
      mem[std::stoi(location)] = std::stoi(toWrite);
      std::cout<<mem[std::stoi(location)];
    }
    if(instruc[0] == 'e')
      break;
    }

    //close and exit process
    close(pipes2[1]);
    close(pipes[0]); 
    exit(0);
  }

  //CPU process
 
  //close pipes that aren't being used
  close(pipes[0]);
  close(pipes2[1]);

  //initialize registers
  int pc = 0;
  int sp = 999;
  int ir = 0;
  int ac = 0;
  int x = 0;
  int y = 0;
  int operand;

  char instruc[11];
  createInstruction(instruc, 'r', pc);

  //TEST WRITING
  createInstruction(instruc, 'w', 4, 100);
  //start CPU by requesting first instruction from memory
  write(pipes[1], instruc, 12);
  read(pipes2[0], &ir, sizeof(ir));

  //execute instructions
  while(ir != 50)
  {
    switch(ir)
    {
      case 1: //load the value into AC
	pc++;
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, 12);
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 2: //load value from the address found into AC
	pc++; 
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, 12); //request address
	read(pipes2[0], &operand, sizeof(operand)); //get address
	createInstruction(instruc, 'r', operand); 
	write(pipes[1], instruc, 12); //request what is at the address
	read(pipes2[0], &ac, sizeof(ac));
	break;
      default: 
	perror("Not a valid command");
	break;
    }
    pc++;
    std::cout<<"IR is " << ir << " and AC is " << ac << " and PC is " << pc << std::endl;
    createInstruction(instruc, 'r', pc);
    write(pipes[1], instruc, 12);
    read(pipes2[0], &ir, sizeof(ir));

  }
  if(ir == 50)
  {
    createInstruction(instruc, 'e', pc);
    write(pipes[1], instruc, 12);
  }

  //close and exit
  close(pipes[1]);
  close(pipes2[0]);
  waitpid(-1, NULL, 0);
  return(0);
}

void createInstruction(char* a, char type, int pc)
{
  memset(a, 0, sizeof(a));
  a[0] = type;
  a[1] = ' ';
  if(pc < 10)
    a[2] = pc + '0';
  else if(pc < 100)
  {
    int tens = pc/10;
    int singles = pc % 10; 
    a[2] = tens + '0';
    a[3] = singles + '0';
  }
  else if(pc < 1000)
  {
    int hundreds = pc/100;
    int tens = (pc%100)/10;
    int singles = (pc%100)%10;
    a[2] = hundreds + '0';
    a[3] = tens + '0';
    a[4] = singles + '0';
  }
  else //pc > 1000
  {
    int thousands = pc/1000;
    int hundreds = (pc%1000)/100;
    int tens = ((pc%1000)%100)/10;
    int singles = ((pc%1000)%100)%10;
    a[2] = thousands + '0';
    a[3] = hundreds + '0';
    a[4] = tens + '0'; 
    a[5] = singles + '0'; 
  }
}

void createInstruction(char* a, char type, int pc, int location)
{

  memset(a, 0, sizeof(a));
  a[0] = type;
  a[1] = ' ';
  if(pc < 10)
    a[2] = pc + '0';
  else if(pc < 100)
  {
    int tens = pc/10;
    int singles = pc % 10; 
    a[2] = tens + '0';
    a[3] = singles + '0';
  }
  else if(pc < 1000)
  {
    int hundreds = pc/100;
    int tens = (pc%100)/10;
    int singles = (pc%100)%10;
    a[2] = hundreds + '0';
    a[3] = tens + '0';
    a[4] = singles + '0';
  }
  else //pc > 1000
  {
    int thousands = pc/1000;
    int hundreds = (pc%1000)/100;
    int tens = ((pc%1000)%100)/10;
    int singles = ((pc%1000)%100)%10;
    a[2] = thousands + '0';
    a[3] = hundreds + '0';
    a[4] = tens + '0'; 
    a[5] = singles + '0'; 
  }

  a[6] = ' ';
  if(location < 10)
    a[7] = location + '0';
  else if(location < 100)
  {
    int tens = location/10;
    int singles = location % 10; 
    a[7] = tens + '0';
    a[8] = singles + '0';
  }
  else if(location < 1000)
  {
    int hundreds = location/100;
    int tens = (location%100)/10;
    int singles = (location%100)%10;
    a[7] = hundreds + '0';
    a[8] = tens + '0';
    a[9] = singles + '0';
  }
  else //pc > 1000
  {
    int thousands = location/1000;
    int hundreds = (location%1000)/100;
    int tens = ((location%1000)%100)/10;
    int singles = ((location%1000)%100)%10;
    a[7] = thousands + '0';
    a[8] = hundreds + '0';
    a[9] = tens + '0'; 
    a[10] = singles + '0'; 
  }
}
