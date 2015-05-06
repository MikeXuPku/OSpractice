#include "copyright.h"
#include "console.h"
#include "synchconsole.h"
#include "system.h"
//#include "synch.h"

// Dummy functions because C++ is weird about pointers to member functions

static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { DEBUG('a', "ReadAvail:readAvail->V()\n"); readAvail->V(); }

static void WriteDone(int arg) { DEBUG('a', "WriteDone:writeDone->V()\n"); writeDone->V(); }

//----------------------------------------------------------------------
// SynchConsole::SynchConsole
//     Initialize the simulation of a hardware console device.
//
//    "readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//    "writeFile" -- UNIX file simulating the display (NULL -> use stdout)
//     "readAvail" is the interrupt handler called when a character arrives
//        from the keyboard
//     "writeDone" is the interrupt handler called when a character has
//        been output, so that it is ok to request the next char be

//        output
//----------------------------------------------------------------------

SynchConsole::SynchConsole(char *readFile, char *writeFile, int callArg)
{
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, callArg);
    readAvail = new Semaphore("read avail", callArg);
    writeDone = new Semaphore("write done", callArg);
    readLock = new Lock("readLock");
    writeLock = new Lock("writeLock");        //printf("init console done\n");
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete readAvail;
    delete writeDone;
    delete readLock;
    delete writeLock;
}

char SynchConsole::SynchGetChar()
{
    char ch;
    readLock->Acquire();
    readAvail->P();
    ch = console->GetChar();
    readLock->Release();
    return ch;
}

void SynchConsole::SynchPutChar(char ch)
{
    writeLock->Acquire();
    console->PutChar(ch);
    writeDone->P();
    writeLock->Release();
}
