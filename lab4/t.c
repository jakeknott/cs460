/*******************************  t.c ****************************/

/*
 * Jacob Knott
 * 11398813
 * CS460
 * Take Home Exam
 */

#include "type.h"
#define LIVE 0
#define DEMO 1

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int mode;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

OFT  oft[NOFT];
PIPE pipe[NPIPE];

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "io.c"
#include "vid.c"
#include "kernel.c"           // YOUR kernel.c file
#include "wait.c"             // YOUR wait.c   file
#include "int.c"              // YOUR int.c    file
#include "load.c"
#include "pipe.c"
#include "timer.c"
#include "kbd.c"


int init()
{
    PROC *p; int i, j;
    color = 0x0C;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
        p->inkmode = 1;
        p->time = 5;
        // clear fd[ ] array of PROC
        for (j=0; j<NFD; j++)
            p->fd[j] = 0;
    }

    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = 0;
    sleepList = 0;

    // initialize all OFT and PIPE structures
    for (i=0; i<NOFT; i++)
        oft[i].refCount = 0;
    for (i=0; i<NPIPE; i++)
        pipe[i].busy = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler(int s) //from usermode, pass ina value to change running prosess to ready instead of running.
{
    if(s == 2) //switching from running to ready from  umode
    {
        running->status = READY;
    }
    if (running->status == READY)
    {
        enqueue(&readyQueue, running);
    }
    running = dequeue(&readyQueue, 0);
    running->status = RUNNING;
    color = running->pid + 7;
}

int int80h();
int kbinth();
int set_vector(u16 vector, u16 addr)
{
     // put_word(word, segment, offset)
     /*put_word(handler, 0, vector*4); // set PC = _int80h
     put_word(0x1000,  0,(vector*4) + 2); // set CS = 0x1000*/
     u16 location,cs;
     location = vector << 2;
     put_word(addr, 0, location);
     put_word(0x1000,0,location+2);
}
            
main()
{
    char m;
    vid_init();
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vector(80, int80h);

    set_vector(9, kbinth);
    kbd_init();

    lock();
    set_vector(8, tinth);
    timer_init();


    mode = LIVE; //used for demoing the time working, set to DEMO to see four procs switch by themselves

    kfork("/bin/u1");     // P0 kfork() P1

    if(mode == DEMO)
    {
        kfork("/bin/u1");
        kfork("/bin/u1");
        kfork("/bin/u1");
    }

    while(1){
        //printf("P0 running\n");
        while(!readyQueue);
            //printf("P0 switch process\n");
            running->status = READY;
            tswitch();         // P0 switch to run P1
   }
}
