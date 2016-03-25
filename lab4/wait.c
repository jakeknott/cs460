/********************** wait.c  ***************************/

int ksleep(int event)
{
    running->event = event; // record event in PROC.event
    running->status = SLEEP; // change status to SLEEP
    enqueue(&sleepList, running);
    tswitch(0);
}

int kwakeup (int event)
{
    int i; PROC *p;
    for (i=1; i<NPROC; i++)
    { // not applicable to P0
        p = &proc[i];
        if (p->status == SLEEP && p->event == event)
        {
            printf("Wakup %d\n", p->pid);
            p->event = 0; // cancel PROC’s event
            p->status = READY; // make it ready to run again>
            dequeue(&sleepList, p);
            enqueue(&readyQueue, p);
        }
    }
}
int kwait(int *status)
{
    PROC *p; int i, hasChild = 0;
    while(1)
    {
        for (i=1; i<NPROC; i++)
        {
            p = &proc[i];
            if (p->status != FREE && p->ppid == running->pid)
            {//find all children procs
                hasChild = 1;
                if (p->status == ZOMBIE){ //if child is a zombie, has exited
                    *status = p->exitCode;
                    p->status = FREE;
                    p->next = 0;
                    put_proc(&freeList, p);
                    nproc--;
                    printf("returning\n");
                    return(p->pid);
                }
            }
        }

        if (!hasChild) return -1;
        ksleep(running); // still has kids alive: sleep on PROC address
    }
}
int do_sleep()
{
    char event[128];
    int eventnum = 0, i = 0;
    for(i = 0; i < 128;i++)
        event[i] = '\0';

    printf("enter event value: ");
    gets(event);
    eventnum = stoi(event);
    printf("\n");
    ksleep(eventnum);
}
int do_wakeup() 
{
    char event[128];
    int eventnum = 0, i = 0;

    for(i = 0; i < 128;i++)
        event[i] = '\0';

    printf("enter event value: ");
    gets(event);
    eventnum = stoi(event);
    printf("\n");
    kwakeup(eventnum);
}
int do_wait()
{
    int pid, status;
    pid = kwait(&status);
    if( pid == -1)
    {
        printf("\n!!!!!!!!!!!!!!  Error! no children  !!!!!!!!!!!!!\n\n");
        return;
    }
    printf("Found ZOMBIE: pid = %d\texit status = %d\n", pid, status);
}
