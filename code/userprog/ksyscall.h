/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"


void Fork(int s){
  printf("Fork the thread!");

}

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysWrite(int addr, int size)
{
  int temp;
  if(size > 0){
    for(int i = 0; i < size; i++){
      //bool ReadMem(int addr, int size, int* value);
      kernel->machine->ReadMem(addr+i, 1, &temp);
      printf("%c", temp);
    }
    printf("SysWrite Done\n");
  }
  return size;
}

int SysRead(int addr, int size){
  char* toRead = "ReadSysCall";
  int holder;
  if(size > 0){
    for(int i = 0; i < size; i++){
      holder = (int)(toRead[i]);
      kernel->machine->WriteMem(addr, 1, holder);
      addr++;
    }
    printf("SysRead Done\n");
  }
}




int SysAdd(int op1, int op2)
{
  return op1 + op2;
}


#endif /* ! __USERPROG_KSYSCALL_H__ */
