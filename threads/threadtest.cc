// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "list.h"

// testnum is set in main.cc
int testnum = 1;
// define the size of buffer in problem producer and consumer
#define p_c_buffer_size 4
List *buffer = new List; 
// define the semaphore we will use in test4
Semaphore P_C_Empty("producer and consumer Empty", p_c_buffer_size);
Semaphore P_C_Full("producer and consumer Full", 0);
Lock P_C_Lock("producer and consumer lock");

void
TS()
{
    printf("***********************************************\n");
    printf("TS command:\n");
    for(int i=0;i < MAX_THREADS; ++i)
    {
        if(thread_slot->getByIndex(i) != NULL){
		Thread * temp = (Thread *)thread_slot->getByIndex(i);
		temp->Print();
	}
    }
    printf("***********************************************\n");
}


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d name %s looped %d times\n", currentThread->getId(),currentThread->getName(), num);
        currentThread->Yield();
    }
    TS();  //insert TS command to inquire threads' information.
}

void SimpleThread2(int dummy)
{
    //do nothing at all
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest2
// 	Set a limit for max threads, then wait for results.
//----------------------------------------------------------------------

void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest1");
    Thread *t1 = new Thread("forked thread 1");
    Thread *t2 = new Thread("forked thread 2");
    Thread *t3 = new Thread("forked thread 3");
    Thread *t4 = new Thread("forked thread 4");
    t1->Fork(SimpleThread, 1);
    t2->Fork(SimpleThread, 2);
    t3->Fork(SimpleThread, 3);
    t4->Fork(SimpleThread, 4);
    //SimpleThread(0);
}

void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest1");
    Thread *t1 = new Thread("No1",5);
    Thread *t2 = new Thread("No2",2);
    Thread *t3 = new Thread("No3",8);
    Thread *t4 = new Thread("No4",9);
    t1->Fork(SimpleThread, 1);
    t2->Fork(SimpleThread, 2);
    t3->Fork(SimpleThread, 3);
    t4->Fork(SimpleThread, 4);
    SimpleThread(0);
}

//-----------------------------------------------------------------
// problem producer and consumer
//-----------------------------------------------------------------
void insert_item(Semaphore *empty, Semaphore *full, List *buff, void *item, int i, Lock *lock)
{
    empty->P();
    lock->Acquire();
    buff->Append(item);
    printf("-------------%dth-----produce %d----------------------\n", i, (int)item);
    lock->Release();
    full->V();
}

void remove_item(Semaphore *empty, Semaphore *full, List *buff, int i, Lock *lock)
{
    full->P();
    lock->Acquire();
    int a = (int)buff->Remove();
    printf("-------------%dth-----consume %d----------------------\n", i, a);
    lock->Release();
    empty->V();
}

void producer(int arg)
{
    for(int i=0; i<10; ++i){
	insert_item(&P_C_Empty, &P_C_Full, buffer, (void*)arg, i, &P_C_Lock);
    }
}

void consumer(int arg)
{
    for(int i=0;i<10; ++i){
	remove_item(&P_C_Empty, &P_C_Full, buffer, i, &P_C_Lock);
    }
}

void
ThreadTest4()
{
    //producer and consumer problem
    //int* buffer = new int[10];  // define the size of buffer
    for(int i=0; i<2;++i){
	Thread *temple = new Thread("producer");
	temple->Fork(producer, i);
    }
    for(int i=0; i<2;++i){
	Thread *temple = new Thread("consumer");
	temple->Fork(consumer, i);
    }
}

//-------------------------------------------------
//barrier problem
//----------------------------------------------
#define barrierThreadNum 4
Barrier barrier("barrier problem", barrierThreadNum );

void barrierThread(int arg)
{
    for(int i=0;i<5;++i){currentThread->Print(); printf("begin to wait for %dth opertion\n", i); barrier.Wait();}
}

void
ThreadTest5() //barrier test
{
    for(int i=0;i<barrierThreadNum;++i){
	Thread *temple =new Thread("barrier thread");
        temple->Fork(barrierThread, i);
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
	ThreadTest2();
	break;
    case 3:
	ThreadTest3();
	break;
    case 4:
	ThreadTest4();
	break;
    case 5:
	ThreadTest5();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

