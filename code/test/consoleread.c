/* consoleread.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"


int
main()
{
  int res = ConsoleRead("Console Write", 13);
  
  //Halt();
  /* not reached */
}
