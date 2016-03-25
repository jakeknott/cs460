/*******************************  kernel.c ****************************/

//#include "io.c"  // include I/O functions based on getc()/putc()

#define NPROC 9
#define SSIZE 1024
/******* PROC status ********/


#define FREE 0
#define READY 1
#define RUNNING 2
#define STOPPED 3
#define SLEEP 4
#define ZOMBIE 5

extern int color = 2;

void printList (char *name, PROC *list)
{
    PROC *p;
    p = list;

    printf("%s = ", name);
    while (p)
    {
        printf("[%d, %d]    ->    ", p->pid, p->priority);
        p = p->next;
    }
    printf("NULL\n");
    return;
}

PROC* get_proc(PROC **list)
{
    PROC *p;
    PROC *t;
    p = *list;

    if(p->status == FREE)//first element
    {
        *list = p->next;
        p->next = 0;
        return p;
    }
    return 0;
}

void put_proc(PROC **list, PROC *p)
{
    PROC *t = *list;

    *list = p;
    p->next = t;
}

void enqueue (PROC **list, PROC *p)
{
    PROC *t = *list;
    PROC *t2;

    //printf("t = %d\n", t);
    if(!t)
    {
        *list = p;
        return;
    }
  
    if(p->priority > t->priority)//put at front
    {
        p->next = t;
        *list = p;
        return;
    }

    while(t->next)
    {
        if(p->priority < t->priority && p->priority > t->next->priority)
	    {
            t2 = t->next;
	        t->next = p;
            p->next = t2;
            return;
 	    }

        if(p->priority <= t->priority && p->priority > t->next->priority)
 	    {
 	        t2 = t->next;
 	        t->next = p;
 	        p->next = t2;
            return;
 	    }
        t = t->next;
    }
   //p must be the lowest priority, put at end
    t->next = p;
    p->next = 0;
}

void printSleep()
{
  PROC *p;
  p = sleepList;
  printf("%s = ", "sleepList");
  while (p)
    {
      printf("[%d, e=%d]    ->    ", p->pid, p->event);
      p = p->next;
    }
  printf("NULL\n");
  return; 
}


PROC* dequeue(PROC **list, PROC *p)
{
    PROC *t, *n;
    t  = *list;
    if (p == 0 || p->pid == t->pid)
    {
        *list = t->next;
        t->next = 0;
        //t->status = RUNNING;
        return t;
    }

    while(t->next)
    {
        if(t->next->pid == p->pid)//remove this entrie
	    {
	        n = t->next;
	        t->next = n->next;
	        n->next = 0;
	    }
    }
    return 0;
}

/*************************kernel commands ***************************/

int kexit(int exitvalue)
{
  int i, wakeupP1 = 0;
  PROC *p;
  printf("exitvalue = %d\n", exitvalue);
  if (running->pid==1 && nproc>2){ // nproc = number of active PROCs
    printf("other procs still exist, P1 can't die yet\n");
    return -1;
  }
  /* send children (dead or alive) to P1's orphanage */
  for (i = 1; i < NPROC; i++){
    p = &proc[i];
    if (p->status != FREE && p->ppid == running->pid){
      p->ppid = 1;
      //p->ppid = &proc[1];
      wakeupP1++;
    }
  }
  running->exitCode = exitvalue;
  running->status = ZOMBIE;
  /* wakeup parent and also P1 if necessary */
  p = &proc[running->ppid]; //get p = running parent
  kwakeup(p->event); // parent sleeps on its PROC address
  if (wakeupP1){
    kwakeup(&proc[1]);}

    for (i=0; i<NFD; i++)
    {
        if (running->fd[i] != 0)
            close_pipe(i);
    }

  tswitch(0);
}

int goUmode();

int body()
{
    char c;
    printf("proc %d starts from body()\n", running->pid);

    running->inkmode = 1;

    while(1){
      printf("------------------------------------------------------\n");
      printList("freelist ", freeList);// optional: show the freeList
      printList("readyQueue", readyQueue); // show the readyQueue
      printSleep();
      printf("------------------------------------------------------\n");
      printf("proc %d running: parent=%d\n",running->pid,running->ppid);
      printf("enter a char [s|f|q| z|a|w|u]: ");
      c = getc(); printf("%c\n", c);
        timeclear(1);
      switch(c)
	    {
	        case 'f' : do_kfork(); break;
	        case 's' : do_tswitch(); break;
        	case 'q' : do_exit(); break;
	        case 'z' : do_sleep(); break;
	        case 'a' : do_wakeup(); break;
	        case 'w' : do_wait(); break;
	        case 'u': printf("running: %x, \n", running->uss); running->inkmode = 0; goUmode(); break;
	    }
    }
}
PROC *kfork(char *filename) // create a child process, begin from body()
{
    int i, j;
    u16 segment;
    PROC *p;

    printf("in kfork\n");
    p = get_proc(&freeList);
    if (!p){
        printf("no more PROC, kfork() failed\n");
        return 0;
    }
    p->status = READY;
    p->priority = 1; // priority = 1 for all proc except P0
    p->ppid = running->pid; // parent = running
    /* initialize new proc's kstack[ ] */
    for (i=1; i<10; i++) // clear saved CPU registers
        p->kstack[SSIZE-i]= 0 ; // all 0's
    p->kstack[SSIZE-1] = (int)goUmode; // resume point=address of body()
    p->ksp = &p->kstack[SSIZE-9]; // proc saved sp

    segment = (p->pid + 1)*0x1000;
    if(filename)
    {
        load(filename, segment);
        for (j=1; j<13; j++)
        {
            put_word(0, segment, -j*2);       // Set all registers to 0
        }
        put_word(0x0200, segment, -2);      // uFlag
        put_word(segment, segment, -4);     // uCS
        put_word(segment, segment, -22);    // uES
        put_word(segment, segment, -24);    // uDS
        p->uss = segment;
        p->usp = -24;

    }

    p->inkmode = 0; //because it is loaded into umode
    enqueue(&readyQueue, p); // enter p into readyQueue by priority

    return p; // return child PROC pointer
}


int copyImage(u16 pseg, u16 cseg, u16 size)
{
    u16 i;
    for (i=0; i<size; i++)
        put_word(get_word(pseg, 2*i), cseg, 2*i);
}

int fork()
{
    int pid, i; u16 segment;
    PROC *p = kfork(0); // kfork() a child, do not load image file
    running->inkmode = 1;
    if (p==0)
    {
        printf("fork failed!\n");
        return -1;
    } // kfork failed
    segment = (p->pid+1)*0x1000; // child segment
    copyImage(running->uss, segment, 32*1024); // copy 32K words
    p->uss = segment; // child’s own segment
    p->usp = running->usp; // same as parent's usp/
    ///*** change uDS, uES, uCS, AX in child's ustack ****
    put_word(segment, segment, p->usp); // uDS=segment
    put_word(segment, segment, p->usp+2);// uES=segment
    put_word(0,segment, p->usp+2*8); // uax=0
    put_word(segment, segment, p->usp+2*10);// uCS=segment

    for (i=0; i<NFD; i++){
        p->fd[i] = running->fd[i];
        if (p->fd[i] != 0){
            p->fd[i]->refCount++;
            if (p->fd[i]->mode == READ_PIPE)
                p->fd[i]->pipe_ptr->nreaders++;
            if (p->fd[i]->mode == WRITE_PIPE)
                p->fd[i]->pipe_ptr->nwriters++;
        }
    }

    running->inkmode = 0;


    return p->pid;
}

int kexec(char *y) // y points at filenmae in Umode space
{
    int i, length = 0;
    char filename[64], *cp = filename;
    u16 segment = running->uss;// same segment
    /* get filename from U space with a length limit of 64 */
    while( (*cp++ = get_byte(running->uss, y++)) && length++ < 64 );
    if (!load(filename, segment)) // load filename to segment
        return -1; // if load failed, return -1 to Umode
    /* re-initialize process ustack for it return to VA=0 */
    for (i=1; i<=12; i++)
        put_word(0, segment, -2*i);
    running->usp = -24; // new usp = -24
    /* -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 ustack layout */
    /* flag uCS uPC ax bx cx dx bp si di uES uDS */
    put_word(segment, segment, -2*12); // saved uDS=segment
    put_word(segment, segment, -2*11); // saved uES=segment
    put_word(segment, segment, -2*2); // uCS=segment; uPC=0
    put_word(0x0200, segment, -2*1); // Umode flag=0x0200
    //cp = "hello\0";
    //printf("cp = %s\n", cp);
    //put_word(cp, segment, (segment + 0x1000) - 2); //place filename at high end of segment

    return 0;
}

int strlen(char s[])
{
  int i = 0, len = 0;

  while (s[i] != '\0')
    {
      len ++;
      i++;
    }
  return len;
}

int stoi (char s[])
{
  int i = 0, result = 0, len = 0;
  len = strlen(s);

  for(i=0; i<len; i++){
    result = result * 10 + ( s[i] - '0' );
  }
  return result;
}


int do_kfork( )
{
    PROC *p;
    char *dfile = "/bin/u1";
    p = kfork(dfile);
    if (p == 0){ printf("kfork failed\n"); return -1; }
    printf("PROC %d kfork a child %d\n", running->pid, p->pid);
    return p->pid;
}
int do_tswitch()
{
    running->status = READY;
    tswitch();
}

int do_exit() 
{
  char exit[128];
  int exitnum = 0, i = 0;

  for(i = 0; i < 128;i++)
    exit[i] = '\0';

  printf("enter exit value: ");
  gets(exit);
  exitnum = stoi(exit);
  printf("\n");
  kexit(exitnum);
}
