#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"
#include "system.h"

// The following class defines a hardware console device.
// Input and output to the device is simulated by reading
// and writing to UNIX files ("readFile" and "writeFile").
// Since the device is asynchronous, the interrupt handler "readAvail"
// is called when a character has arrived, ready to be read in.
// The interrupt handler "writeDone" is called when an output character
// has been "put", so that the next character can be written.

class SynchConsole {
  public:
    SynchConsole(char *readFile, char *writeFile, int callArg);     // initialize the hardware console device
    ~SynchConsole();            // clean up console emulation

    void SynchPutChar(char ch);    
    char SynchGetChar();      

  private:
    Console *console;
    Lock *readLock;
    Lock *writeLock;
};