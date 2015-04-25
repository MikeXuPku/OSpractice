// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#define FREE 1
#define BUSY 0

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    //printf("%s:P()\n",name);
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
        //currentThread->Print();
	//printf("goto sleep by %s:P()\n",name);
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
//printf(" %s:V()\n",name);
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) 
{
    name = debugName;
    lock = new Semaphore(debugName, 1);
    owner = NULL;
}

Lock::~Lock() 
{
    delete lock;
}

void Lock::Acquire() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    lock->P();
    owner = currentThread;
    (void)interrupt->SetLevel(oldLevel);
}

void Lock::Release()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(currentThread == owner);
    owner = NULL;
    lock->V();
    (void) interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread(){return currentThread == owner;}



Condition::Condition(char* debugName) 
{
    name = debugName;
    queue = new List;
}

Condition::~Condition() 
{
    delete queue;
}

void Condition::Wait(Lock* conditionLock)
{
    DEBUG('t', "...conition->Wait() is calling...n");
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    ASSERT(conditionLock->isHeldByCurrentThread());    //check the prerequisit
  
    if(queue->IsEmpty()) lock = conditionLock;

    ASSERT(lock == conditionLock);

    DEBUG('t', "condition->Wait(): Apppend the \"%s\" thread into \"%s\"->queue and go to sleep!\n", currentThread->getName(), name);
    queue->Append((void*)currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);
}
void Condition::Signal(Lock* conditionLock)
{
    Thread *thread;
    DEBUG('t', "...condition->Signal() is calling...\n");
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    if(!queue->IsEmpty()){
	ASSERT(lock == conditionLock);
	thread = (Thread*)queue->Remove();
	scheduler->ReadyToRun(thread); // wake up the thread
    }
    (void) interrupt->SetLevel(oldLevel);
}
void Condition::Broadcast(Lock* conditionLock)
{
    Thread *thread;
    DEBUG('t', "...condition->Broadcast() is calling...\n");
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    while(!queue->IsEmpty()){
	ASSERT(lock == conditionLock);
	thread = (Thread*)queue->Remove();
	scheduler->ReadyToRun(thread); // wake up the thread
    }
    (void) interrupt->SetLevel(oldLevel);    
}

//----------------------------------------------------------
Barrier::Barrier(char *debugName, int cnt){
    name = debugName;
    max_count = cnt;
    count = 0;
    lock = new Lock(debugName);
    condition = new Condition(debugName);
}
Barrier::~Barrier(){
    delete lock;
    delete condition;
}
void Barrier::Wait(){
    lock->Acquire();
    count += 1;
    if(count == max_count){count = 0; condition->Broadcast(lock);}
    else condition->Wait(lock);
    lock->Release();
}

//------------------------------------------------------------------
RWLock::RWLock(char *debugName){
    mutex = new Semaphore(debugName, 1);
    rc = 0;
    w = new Semaphore(debugName, 1);
}

RWLock::~RWLock(){
    delete mutex;
    delete w;
}
void RWLock::RLock(){
    mutex->P();
    rc += 1;
    if(rc == 1) w->P();
    mutex->V();
}
void RWLock::RRelease(){
    mutex->P();
    rc -= 1;
    if(rc == 0) w->V();
    mutex->V();
}
void RWLock::WLock(){
    w->P();
}
void RWLock::WRelease(){
    w->V();
}
