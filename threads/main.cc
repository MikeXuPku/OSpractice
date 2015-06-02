// main.cc 
//	Bootstrap code to initialize the operating system kernel.
//
//	Allows direct calls into internal operating system functions,
//	to simplify debugging and testing.  In practice, the
//	bootstrap code would just initialize data structures,
//	and start a user program to print the login prompt.
//
// 	Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//		-s -x <nachos file> -c <consoleIn> <consoleOut>
//		-f -cp <unix file> <nachos file>
//		-p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system 
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"

#ifdef THREADS
extern int testnum;
#endif

// External functions used by this file

extern void ThreadTest(int), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void), filesysTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out), SynchConsoleTest(char *in, char *out);
extern void MailTest(int networkID);

void startProcess(int a){
	StartProcess((char*)(a));
}
//----------------------------------------------------------------------------------------------------
//shell function
//----------------------------------------------------------------------------------------------------
void shell(int dummy = 0){
	char *p = "nachos@nachos:~";
	char currentD[200];
	strncpy(currentD, p, 20);
	char command[100];
	char str1[100];
	char str2[100];
	printf("%s%c", currentD, '$');
	while(1){
		gets(command);
		if(strcmp(command, "quit") == 0)break;                    //single command
		else if(strcmp(command, "ls") == 0){
			fileSystem->ls();
			printf("\n");
		}
		else if(strcmp(command, "threadTest") == 0){
					Thread * thread = new Thread("threadTest");
					int id = thread->Fork(ThreadTest, 1);
                				thread->uid_ = currentThread->getId();   //work equal to join
                				currentThread->Sleep();
		}
		else{                                                                                       //dual command
			int num = sscanf(command, "%s%s", str1, str2);
			if(num == -1)printf("invalid command!\n");
			else{
				if(strcmp(str1, "create") == 0){
					fileSystem->Create(str2, 10);                         //default file size
				}
				else if(strcmp(str1, "mkdir") == 0){
					fileSystem->mkdir(str2);
				}
				else if(strcmp(str1, "delete") == 0){
					fileSystem->Remove(str2);
				}
				else if(strcmp(str1, "file") == 0){
					fileSystem->file(str2);
				}
				else if(strcmp(str1, "cd") == 0){
					if(fileSystem->CD(str2)){
						if(strcmp(str2, "..") == 0){
							int length = strlen(currentD);
							//printf("cd .. OK\n");
							for(int i = length;i>=0;--i){
								if(currentD[i] == '/'){
									//printf("%s\n", currentD);
									currentD[i] = '\0';
									//printf("%s\n", currentD);
									break;
								}
							}
						}
						else{
							int length = strlen(currentD);
							currentD[length] = '/';
							strncpy(currentD+length+1, str2, 20);
						}
					}
				}
				else if(strcmp(str1, "exec") == 0){					
					Thread * thread = new Thread(str2);
					int id = thread->Fork(startProcess, (int)str2);
                				thread->uid_ = currentThread->getId();   //work equal to join
                				currentThread->Sleep();

				}
				else{
					printf("invalid command\n" );
				}
			}
		}
		printf("%s%c", currentD, '$');
	}
}

//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
    int argCount;			// the number of arguments 
					// for a particular command

    DEBUG('t', "Entering main");
    (void) Initialize(argc, argv);
    
#ifdef THREADS
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
      argCount = 1;
      switch (argv[0][1]) {
      case 'q':
        testnum = atoi(argv[1]);
        argCount++;
        break;
      default:
        testnum = 1;
        break;
      }
    }
    ThreadTest();
#endif

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
        if (!strcmp(*argv, "-z"))               // print copyright
            printf (copyright);
#ifdef USER_PROGRAM
        if (!strcmp(*argv, "-x")) {        	// run a user program
	    ASSERT(argc > 1);
	    if(strcmp(*(argv + 1), "shell") == 0){
	    	shell();
	    	break;
	    }
            StartProcess(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-c")) {      // test the console
	    if (argc == 1)
	        ConsoleTest(NULL, NULL);
	    else {
		ASSERT(argc > 2);
	        ConsoleTest(*(argv + 1), *(argv + 2));
	        argCount = 3;
	    }
	    interrupt->Halt();		// once we start the console, then 
					// Nachos will loop forever waiting 
					// for console input
	}
	else if(!strcmp(*argv, "-synchc")){
		if(argc == 1)SynchConsoleTest(NULL, NULL);
		else{
			ASSERT(argc > 2);
			SynchConsoleTest(*(argv+1), *(argv+2));
			argCount = 3;
		}
		interrupt->Halt();
	}
#endif // USER_PROGRAM
#ifdef FILESYS
	printf("we defned the fileSystem!!!!!!!!!!!!!!!!!!!\n");
	if (!strcmp(*argv, "-cp")) { 		// copy from UNIX to Nachos
	    ASSERT(argc > 2);
	    Copy(*(argv + 1), *(argv + 2));
	    argCount = 3;
	} else if (!strcmp(*argv, "-p")) {	// print a Nachos file
	    ASSERT(argc > 1);
	    Print(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-r")) {	// remove Nachos file
	    ASSERT(argc > 1);
	    fileSystem->Remove(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-l")) {	// list Nachos directory
		printf("ready to print fileSystem context:\n");
            fileSystem->List();
	} else if (!strcmp(*argv, "-D")) {	// print entire filesystem
            fileSystem->Print();
	} else if (!strcmp(*argv, "-t")) {	// performance test
            filesysTest();
	}
#endif // FILESYS
#ifdef NETWORK
        if (!strcmp(*argv, "-o")) {
	    ASSERT(argc > 1);
            Delay(2); 				// delay for 2 seconds
						// to give the user time to 
						// start up another nachos
            MailTest(atoi(*(argv + 1)));
            argCount = 2;
        }
#endif // NETWORK
    }
    currentThread->Finish();	// NOTE: if the procedure "main" 
				// returns, then the program "nachos"
				// will exit (as any other normal program
				// would).  But there may be other
				// threads on the ready list.  We switch
				// to those threads by saying that the
				// "main" thread is finished, preventing
				// it from returning.
    return(0);			// Not reached...
}
