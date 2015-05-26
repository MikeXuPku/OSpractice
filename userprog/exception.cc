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
void runUserThread(int addr){
    AddrSpace * space = currentThread->space;
    TranslationEntry* newpageTable = new TranslationEntry[space->numPages + 2];
    for(int i=0;i<space->numPages;++i)newpageTable[i] = space->pageTable[i];
    for(int i=space->numPages;i<2+ space->numPages;++i){
        
            newpageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
            newpageTable[i].physicalPage = -1;
            newpageTable[i].valid = false;
            newpageTable[i].use = FALSE;
            newpageTable[i].dirty = FALSE;
            newpageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
                    // a separate page, we could set its 
                    // pages to be read-only
            newpageTable[i].time_when_use = 0;
    }
    delete space->pageTable;
    space->pageTable = newpageTable;
    space->numPages = space->numPages +2;

    space->InitRegisters();     // set the initial register values
    machine->WriteRegister(PCReg, addr);
    machine->WriteRegister(NextPCReg, addr +4);
    space->RestoreState();      // load page table register

    machine->Run();         // jump to the user progam
    ASSERT(FALSE);               
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {                                                       // Halt
	DEBUG('a', "Shutdown, initiated by user program.\n");
	printf("normal shutdown by calling systemcall sc_halt\n");
	printf("tlb hit times: %d\n", tlbhit);
	printf("tlb miss times: %d\n", tlbmiss);
	printf("tlb hit rate: %f\%\n", (tlbhit*100.0)/(float)(tlbhit + tlbmiss));
   	interrupt->Halt();
    } 
    else if(which == SyscallException && type == SC_Create){
                printf("syscall to create a file\n");
                int base = (int)machine->ReadRegister(4);
                int value, count;
                for(count = 0;true ;count++){
                        machine->ReadMem(base + count, 1, &value);
                        if(value == 0)break;
                }
                char *filename = new char[count+1];
                for(int i = 0; i <= count; ++i){
                        machine->ReadMem(base + i, 1, (int *)(filename + i));
                }
                printf("the filename you want to create is: %s\n", filename);
                fileSystem->Create(filename, 100);
                printf("create file successfully!!!!!!!!\n");
                machine->addPC();
                delete []filename;
    }
    else if(which == SyscallException && type == SC_Open){
       #ifdef FILESYS_STUB 
                printf("syscall to open a file\n");
                int base = (int)machine->ReadRegister(4);
                int value, count;
                for(count = 0;true ;count++){
                        machine->ReadMem(base + count, 1, &value);
                        if(value == 0)break;
                }
                char *filename = new char[count+1];
                for(int i = 0; i <= count; ++i){
                        machine->ReadMem(base + i, 1, (int *)(filename + i));
                }
                printf("the filename you want to open is: %s\n", filename);
                int fileId = (fileSystem->Open(filename))->file;
                printf("open file successfully!!!!!!!!\n");
                machine->WriteRegister(2, fileId);   //return value
                machine->addPC();
                delete []filename;
        #else
                printf("syscall to open a file\n");
                int base = (int)machine->ReadRegister(4);
                int value, count;
                for(count = 0;true ;count++){
                        machine->ReadMem(base + count, 1, &value);
                        if(value == 0)break;
                }
                char *filename = new char[count+1];
                for(int i = 0; i <= count; ++i){
                        machine->ReadMem(base + i, 1, (int *)(filename + i));
                }
                printf("the filename you want to open is: %s\n", filename);
                // when use our own filesystem, we don't have the fileId, so we return the fhdr sector number 
                int fileId = (fileSystem->Open(filename))->sector_;
                printf("open file successfully!!!!!!!!\n");
                machine->WriteRegister(2, fileId);   //return value
                machine->addPC();
                delete []filename;               

        #endif
    }
    else if(which == SyscallException && type == SC_Close){
        #ifdef  FILESYS_STUB
                printf("syscall to close a file\n");
                int fileId = machine->ReadRegister(4);
                printf("fileId %d to be closed\n",fileId);
                Close(fileId);
                printf("closed file successfully!!!!!!!!!!!\n");
                machine->addPC();
        #else
                printf("syscall to close a file\n");
                int fileId = machine->ReadRegister(4);
                printf("fileId %d to be closed\n",fileId);
                Close(fileId);
                printf("closed file successfully!!!!!!!!!!!\n");
                machine->addPC();
        #endif
    }
    else if(which == SyscallException && type == SC_Write){
                printf("syscall to write a file\n");
                int base = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                int fileId = machine->ReadRegister(6);
                char *content = new char[size +1];
                for(int i=0;i<size;++i){
                    machine->ReadMem(base+i, 1, (int *)(content + i));
                    printf("%c", content[i]);
                }
                content[size] = '\0';
                printf("\nsize is %d. content is: %s\n", size, content);
                OpenFile *point = new OpenFile(fileId);
                point->Write(content, size);    
                delete point;
                delete []content;
                machine->addPC();
                printf("successfully write a file\n");
    }
    else if(which == SyscallException && type == SC_Read){
                printf("syscall to read a file\n");
                int base = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                int fileId = machine->ReadRegister(6);
                char *content = new char[size +1];
                OpenFile *point = new OpenFile(fileId);
                int reNum = point->Read(content, size);
                for(int i=0;i<reNum;++i){
                    machine->WriteMem(base +i, 1, content[i]);
                }  
                content[reNum] = '\0';
                printf("syscall read content: %s\n", content);
                machine->WriteRegister(2, reNum);             
                delete content;
                delete point;
                machine->addPC();
                printf("successfully read syscall!!!!!!!!!!!!!!!!!\n");
    }
    else if(which == SyscallException && type == SC_Exec){
                printf("syscall to exec a userProgram\n");
                int base = (int)machine->ReadRegister(4);
                int value, count;
                for(count = 0;true ;count++){
                        machine->ReadMem(base + count, 1, &value);
                        if(value == 0)break;
                }
                char *filename = new char[count+1];
                for(int i = 0; i <= count; ++i){
                        machine->ReadMem(base + i, 1, (int *)(filename + i));
                }
                printf("the userProgram you want to open is: %s\n", filename);
                OpenFile *executable = fileSystem->Open(filename);
                AddrSpace *space;
                if(executable == NULL){
                    printf("fail to open the file: %s\n", filename);
                    machine->addPC();
                    return;
                }
                space = new AddrSpace(executable);
                printf("new a addrspace successfully!\n");
                //delete currentThread->space;   
                currentThread->space = space;
                space->InitRegisters();
                space->RestoreState();
        #ifdef   FILESYS_STUB
                Close(executable->file);
        #endif
                delete executable;
                printf("ready to exec a new userProgram\n");
                machine->Run();
                ASSERT(false);
    }
    else if(which == SyscallException && type == SC_Join){
                //set the uid of the thread you want to join your tid
                //then sleep
                //read the exit status from the uid of the currentThread
                // we need to modify thread::finish() to let the thread notify his father thread when is finishing
                printf("ready to syscall join\n");
                int id = machine->ReadRegister(4);
                printf("the thread id you want to join is %d\n", id);
                Thread *son = (Thread *)thread_slot->getByIndex(id);
                son->uid_ = currentThread->getId();
                currentThread->Sleep();
                machine->WriteRegister(2, currentThread->exit_id);
                machine->addPC();
                printf("successfullly join. the exit id is %d\n", currentThread->exit_id);
    }
    else if(which == SyscallException && type == SC_Yield){
                printf("ready to syscall yield\n");
                currentThread->Yield();
                machine->addPC();
    }
    else if(which == SyscallException && type == SC_Exit){
               printf("thread exit with status :  %d\n",machine->ReadRegister(4));
               currentThread->exit_id = machine->ReadRegister(4);
               currentThread->Finish();
               machine->addPC();
    }
    else if(which == SyscallException && type == SC_Fork){
                printf("ready to syscall fork\n");
                int addr = machine->ReadRegister(4);
                Thread * newthread = new Thread("syscall fork thread");
                newthread->space = currentThread->space;
                int tid = newthread->Fork(runUserThread, addr);
                machine->WriteRegister(2,tid);
                machine->addPC();
    }
    else if(which == SyscallException && type == SC_Print){
             int p = (int)machine->ReadRegister(4);
             printf("systemcall  Print**********%d\n", p);
             machine->addPC();
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
