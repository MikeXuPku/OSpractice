#include "syscall.h"

void hehe(){
	Print(2);
	Exit(1);
}

int
main()
{
	int id;
    id = Fork(hehe);
    Join(id);
    Print(1);
    Exit(1);		/* and then we're done */
 //Halt();
}