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
#include <ctime>
#include <cmath>
#include <cstdlib>

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

      if(elems[0].compare("r") == 0)
      {
	std::string memoryCells = str.substr(2,str.length()-1);
	write(pipes2[1], &mem[std::stoi(memoryCells)], 2);
      }
      else if(elems[0].compare("w") == 0)
      {
	std::string toWrite = elems[1];
	std::string location = elems[2];
	std::cout<< "Writing" << toWrite << " to " << location << std::endl;
	mem[std::stoi(location)] = std::stoi(toWrite);
	std::cout<<mem[std::stoi(location)]<<std::endl;
	write(pipes2[1], "good", 5);
      }
      else if(elems[0].compare("e") == 0)
      {
	break;
      }
      elems.clear();
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

  //seeds time for random function (Instruction 8)
  srand(time(NULL));

  //stores the instructions
  char instruc[30];

  //buffer to confirm if writing has happened
  char confirm[30];

  //start CPU by requesting first instruction from memory
  createInstruction(instruc, 'r', pc);
  write(pipes[1], instruc, sizeof(instruc));
  read(pipes2[0], &ir, sizeof(ir));

  //execute instructions
  while(ir != 50)
  {
    switch(ir)
    {
      case 1: //load the value into AC
	pc++;
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc));
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 2: //load value from the address found into AC
	pc++; 
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc)); //request address
	read(pipes2[0], &operand, sizeof(operand)); //get address
	createInstruction(instruc, 'r', operand); 
	write(pipes[1], instruc, sizeof(instruc)); //request what is at the address
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 3: //load value from the address found in the given address into the AC
	pc++; 
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc)); //request first address
	read(pipes2[0], &operand, sizeof(operand)); //get first address
	createInstruction(instruc, 'r', operand);
	write(pipes[1], instruc, sizeof(instruc)); //request second address
	read(pipes2[0], &operand, sizeof(operand)); //get second address;
	createInstruction(instruc, 'r', operand); //request what is at the second address
	write(pipes[1], instruc, sizeof(instruc));
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 4: //load value at address+X into the AC
	pc++;
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc)); //request address
	read(pipes2[0], &operand, sizeof(operand)); //get address
	createInstruction(instruc, 'r', operand+x); //request what is at address+x
	write(pipes[1], instruc, sizeof(instruc));
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 5: //load value at address+Y into the AC
	pc++;
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc)); //request address
	read(pipes2[0], &operand, sizeof(operand)); //get address
	createInstruction(instruc, 'r', operand+y); //request what is at address+x
	write(pipes[1], instruc, sizeof(instruc));
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 6: //load from sp+x into the AC
	operand = sp + x;
	createInstruction(instruc, 'r', operand);
	write(pipes[1], instruc, sizeof(instruc));
	read(pipes2[0], &ac, sizeof(ac));
	break;
      case 7: //store value in the AC into the address
	pc++;
	createInstruction(instruc, 'r', pc); 
	write(pipes[1], instruc, sizeof(instruc)); //request address
	read(pipes2[0], &operand, sizeof(operand));
	createWrite(instruc, 'w', ac, operand);
	write(pipes[1], instruc, sizeof(instruc)); 
	read(pipes2[0], &confirm, sizeof(confirm)); //wait for memory to confirm write
	break;
      case 8: //get random int from 1-100 into the AC
	ac = rand() % 100 + 1;
	break;
      case 9: //put port, if port = 1, writes AC as int to screen, if port = 2, writes AC as char
	pc++;
	createInstruction(instruc, 'r', pc);
	write(pipes[1], instruc, sizeof(instruc)); 
	read(pipes2[0], &operand, sizeof(operand));
	c = ac;
	if(operand == 1)
	  std::cout << ac; 
	else if (operand == 2)
	  std::cout << static_cast<char>(ac);
	break;
      default: 
	perror("Not a valid command");
	break;
    }
    pc++;
    std::cout<<"IR is " << ir << " and AC is " << ac << " and PC is " << pc << std::endl;
    createInstruction(instruc, 'r', pc);
    write(pipes[1], instruc, sizeof(ir));
    read(pipes2[0], &ir, sizeof(ir));
  }
  if(ir == 50)
  {
    createInstruction(instruc, 'e', pc);
    write(pipes[1], instruc, sizeof(instruc));
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
  a[ins.length()] = 0;
}

void createWrite(char* a, char type, int pc, int location)
{
  memset(a, 0, 30);
  std::string s = std::string() + type;
  std::string ins = s + " " + std::to_string(pc) + " " + std::to_string(location);
  strncpy(a, ins.c_str(), 30);
  a[ins.length()] = 0;
}

