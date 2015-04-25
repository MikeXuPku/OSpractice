/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
#define scalar   10

int A[scalar];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < scalar; i++)		
        A[i] = scalar - i;
   //Exit(A[9]);
    /* then sort! */
    for (i = 0; i < scalar-1; i++)
        for (j = i; j < (scalar - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
                        //Print(j);
                        //Print(A[j]);
    	   }
     Exit(A[0]);
    //Halt();		/* and then we're done -- should be 0! */
}
