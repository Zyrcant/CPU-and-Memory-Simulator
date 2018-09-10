#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <sstream>

//helper functions to create instructions
void createInstruction(char* a, char type, int pc);
void createWrite(char* a, char type, int pc, int location);

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

      //split instruction by space
      std::stringstream ss(str);
      std::string spot;
      std::vector<std::string> elems;
      while(std::getline(ss, spot, ' '))
      {
	elems.push_back(spot);
      }

      for(int i = 0; i < elems.size(); i++)
	std::cout<<elems[i] + " ";
      std::cout<<std::endl;

      if(elems[0].compare("r") == 0)
      {
	std::string memoryCells = elems[1];
	write(pipes2[1], &mem[std::stoi(memoryCells)], 2);
      }
      else if(elems[0].compare("w") == 0)
      {
	std::string toWrite = elems[1];
	std::string location = elems[2];
	std::cout<< "Writing" << toWrite << " to " << location << std::endl;
	mem[std::stoi(location)] = std::stoi(toWrite);
	std::cout<<mem[std::stoi(location)];
	write(pipes2[1], "good", 5);
      }
      else if(elems[0].compare("e") == 0)
      {
	break;
      }
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


  char instruc[30];

  //buffer to confirm if writing has happened
  char confirm[30];

  /*
  createWrite(instruc, 'w', 300, 1999);
  write(pipes[1], instruc, 31);
  read(pipes2[0], &confirm, sizeof(confirm));*/

  createInstruction(instruc, 'r', pc);

  //start CPU by requesting first instruction from memory
  write(pipes[1], instruc, 31);
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
  memset(a, 0, 30);
  std::string s = std::string() + type;
  std::string ins = s + " " + std::to_string(pc);
  strncpy(a, ins.c_str(), 30);
  a[ins(length()] = 0;
}

void createWrite(char* a, char type, int pc, int location)
{
  memset(a, 0, 30);
  std::string s = std::string() + type;
  std::string ins = s + " " + std::to_string(pc) + " " + std::to_string(location);
  strncpy(a, ins.c_str(), 30);
  std::cout<<ins.length(); 
  a[ins.length()] = 0;
}

