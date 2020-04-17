/* exec.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{
  char* exec_name = "../test/consolewrite";
  int i ;
  for(i = 0; i < 5; i++){
    Exec(exec_name);
  }

  
  //Halt();
  /* not reached */
}
