/* fork.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 */
#include "syscall.h"

// int
// main()
// {
//     Fork(2);
//     Halt();

// }
int
main()
{
    int x;
    x = Fork();
    
    return 0;
}