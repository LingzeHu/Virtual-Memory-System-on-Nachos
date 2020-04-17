/* test.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{
  ConsoleRead("Console Read", 13);
  ConsoleWrite("Console Write", 13);

  /* not reached */
}
