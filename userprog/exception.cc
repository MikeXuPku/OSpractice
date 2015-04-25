// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
	printf("normal shutdown by calling systemcall sc_halt\n");
	printf("tlb hit times: %d\n", tlbhit);
	printf("tlb miss times: %d\n", tlbmiss);
	printf("tlb hit rate: %f\%\n", (tlbhit*100.0)/(float)(tlbhit + tlbmiss));
   	interrupt->Halt();
    } 
    else if(which == SyscallException && type == SC_Exit){
               printf("thread exit with status :  %d\n",machine->ReadRegister(4));
               interrupt->Halt();
    }
    else if(which == SyscallException && type == SC_Print){
             int p = (int)machine->ReadRegister(4);
             printf("systemcall  Print**********%d\n", p);
    }
    else if(which == PageFaultException){
        printf("a PageFaultException found\n");

        //if we come across a pagefault, we need to look at swapfile to find the virtual pages,and load them
        //into main Memory . it means we need allocate a physical page for the virtual page.
        // OK first we need to find the page content in swapfile.
        int bad_va = machine->registers[BadVAddrReg];
        int bad_vpn = bad_va / PageSize;
        int bad_offset = bad_va % PageSize;
        int swap_id = machine->findSwapPage(currentThread->getId(), bad_vpn);
        if(swap_id == -1){
            //printf("OK, in a pagefault handler, we find nothing we can load from swapfile\n");
            // we know this must be a page  for user stack space.then we will allocate a empty physical page to it
            swap_id = machine->findFreeSwapPage();
            machine->swapTable[swap_id].valid =true;
            machine->swapTable[swap_id].virtualPage = bad_vpn;
            machine->swapTable[swap_id].thread_ID = currentThread->getId();
            machine->swapTable[swap_id].dirty =false;
            machine->swapTable[swap_id].use = true;
            machine->swapTable[swap_id].time_when_use = stats->totalTicks;
            machine->swapTable[swap_id].readOnly = false;
        }
        // now we find the virtual page in swapfile, next we need to load it into mainmemory
        //first we try to look at the mainMemory to find a place for new virtualPage.
        int ppn = machine->spareOutPage();
        //load 
        for(int i = 0; i < PageSize; ++i){
            machine->mainMemory[ppn * PageSize + i] = machine->swapfile[swap_id*PageSize+ i];
        }

        (machine->swapTable[swap_id]).valid = false; //release the swapfile resource
        //modify the pagetable;
        machine->pageTable[bad_vpn].virtualPage = bad_vpn;
        machine->pageTable[bad_vpn].physicalPage = ppn;
        machine->pageTable[bad_vpn].valid = true;
        machine->pageTable[bad_vpn].dirty = machine->swapTable[swap_id].dirty;
        machine->pageTable[bad_vpn].readOnly = machine->swapTable[swap_id].readOnly;
        //bool readOnly, dirty   item do not modify ,remain the previous state
        machine->pageTable[bad_vpn].use = true;
        machine->pageTable[bad_vpn].time_when_use = stats->totalTicks;
        
        //set the physical page table
        machine->physTable[ppn].valid = true;
        machine->physTable[ppn].thread_ID = currentThread->getId();
        machine->physTable[ppn].virtualPage = bad_vpn;
        machine->physTable[ppn].time_when_use = stats->totalTicks;
    }
    else{
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
