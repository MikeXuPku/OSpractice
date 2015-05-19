/*#include "syscall.h"
char sentence[19]={};
int main()
{
    int id;
    Create("leo");
    id = Open("leo");
    Write("leo is a big fool!", 18, id);
    int num = Read(sentence, 3, id);
	//printf("successfully read %d bytes\n");
	//printf("my name is %s\n", sentence);
    Close(id);
    Exit(num);
    /* not reached */
//}

/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
 char sentence[20];

int
main()
{
    int i, num;
    num =0;
    Create("leo");
    i = Open("leo");
    Write("leo is a big fool!", 18, i);
    num = Read(sentence, 5, i);
    Close(i);

    Exec("../test/halt");
    

    Exit(num);		/* and then we're done */
 //Halt();
}