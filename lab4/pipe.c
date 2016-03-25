void show_pipe(PIPE *p)
{
    int i, j;
    printf("------------ PIPE CONTENETS ------------\n");
    for(i = 0; i < PSIZE; i++)
    {
        if(p->buf[i] != 0)
            printf("%c", p->buf[i]);
    }
    printf("\nRoom: %d         Data: %d", p->room, p->data);
    printf("\n----------------------------------------\n");


}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
    int i = 0;

    printf(" fd    refCount   mode  \n");
    printf("==========================\n");

    while(i < NFD)
    {
        if(running->fd[i]!=0)
        {
            printf("  %d       %d       ", i, running->fd[i]->refCount);
            if(running->fd[i]->mode == READ_PIPE)
                printf("READ");
            else if (running->fd[i]->mode == WRITE_PIPE)
                printf("WRITE");
            printf("\n");
        }
        i++;
    }

    printf("==========================\n");

}


int read_pipe(int fd, char *buf, int n)
{
    int r = 0;
    PIPE *p;
    OFT *oft;
    u8 byte;
    char *offset;

    offset = buf;

    if(n <= 0)
        return;

    if(fd > NFD || fd < 0)
        return;

    oft = running->fd[fd];
    p = oft->pipe_ptr;

    if(oft->mode != READ_PIPE)
    {
        printf("FD %d is not a read pipe\n", fd);
        return ;
    }

    while(n)
    {
        while(p->data)
        {
            byte = p->buf[p->tail];
            put_byte(byte, running->uss, offset);
            p->buf[p->tail] = 0; //after reading set to null
            n--; r++; p->data--; p->room++;
            offset++;
            p->tail++;
            p->tail %= PSIZE;
            if (n == 0)
                break;
        }
        if(r)
        {
            kwakeup(&p->room);
            show_pipe(p);
            return r;
        }
        //pipe has no data
        if(p->nwriters)
        {
            kwakeup(&p->room);
            ksleep(&p->data);
            continue;
        }
        return 0;
    }
}

int write_pipe(int fd, char *buf, int n)
{
    int r;
    PIPE *p;
    OFT *oft;
    u8 byte;
    char *offset;

    offset = buf;

    if(n <= 0)
        return;

    if(fd > NFD || fd < 0)
        return;

    oft = running->fd[fd];
    p = oft->pipe_ptr;

    if(oft->mode != WRITE_PIPE)
    {
        printf("FD %d is not a write pipe\n", fd);
        return ;
    }

    while(n)
    {
        if(!p->nreaders)
        {
            printf("Proc %d BROKEN_PIPE Error\n", running->pid);
            kexit(BROKEN_PIPE); //no readers dead pipe
        }

        while(p->room)
        {
            byte = get_byte(running->uss, offset);
            p->buf[p->head] = byte;
            p->head++;
            offset++;
            r++; n--; p->data++; p->room--;
            p->head %= PSIZE;
            if(n == 0)
                break;
        }
        kwakeup(&p->data);
        if(n==0)
        {
            show_pipe(p);
            return r;
        }
        printf("going to sleep on room\n");
        ksleep(&p->room);
    }
}

PIPE* getPipe()
{
    int i = 0;

    for(i = 0; i < NPIPE; i++)
    {
        if(pipe[i].busy == 0)
            return &pipe[i];
    }
    printf("No free pipes\n");
    return 0;
}

int getOFT(OFT **read, OFT **write)
{
    int i = 0, j = 0;

    while(i < NOFT)
    {
        if(oft[i].refCount == 0)
        {
            printf("oft[%d]: %x\n", i, &oft[i]);
            *read = &oft[i];
            oft[i].refCount++;
            break;
        }
        i++;
    }

    printf("i = %d\n", i);


    while (i < NOFT)
    {
        if(oft[i].refCount == 0)
        {
            printf("oft[%d]: %x\n", i, &oft[i]);
            *write = &oft[i];
            oft[i].refCount++;
            break;
        }
        i++;
    }

    printf("i = %d\n", i);
    return 1;
}

int kpipe(int pd[2])
{
    // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors

    OFT *rOFT, *wOFT;
    PIPE *np;
    u16 i = 0;

    np = getPipe();
    if(np == 0) //no more pipes
    {
        printf("error: no more pipes\n");
        return 0;
    }

    np->head= 0;
    np->tail=0; np->data=0; np->room=PSIZE; np->nreaders= 1; np->nwriters=1;

    if(getOFT(&rOFT, &wOFT) == 0)
    {
        printf("getOFT failed\n");
        return 0;
    }

    rOFT->mode = READ_PIPE; rOFT->refCount = 1;
    wOFT->mode = WRITE_PIPE; wOFT->refCount = 1;

    rOFT->pipe_ptr = np;
    wOFT->pipe_ptr = np;

    i = 0;
    while(running->fd[i] != 0)
        i++;
    running->fd[i] = rOFT;
    printf("putting %d at pd[0]\n", i);
    put_word(i, running->uss, &pd[0]);

    while(running->fd[i] != 0)
        i++;
    running->fd[i] = wOFT;
    printf("putting %d at pd[1]\n", i);
    put_word(i, running->uss, &pd[1]);

    return 0;
}

int close_pipe(int fd)
{
    OFT *op; PIPE *pp;

    printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

    op = running->fd[fd];
    running->fd[fd] = 0;                 // clear fd[fd] entry

    if (op->mode == READ_PIPE){
        pp = op->pipe_ptr;
        pp->nreaders--;                   // dec n reader by 1

        if (--op->refCount == 0){        // last reader
            if (pp->nwriters <= 0){         // no more writers
                pp->busy = 0;             // free the pipe
                return;
            }
        }
        kwakeup(&pp->room);               // wakeup any WRITER on pipe
        return;
    }

    // YOUR CODE for the WRITE_PIPE case:
    if (op->mode == WRITE_PIPE)
    {
        pp = op->pipe_ptr;
        pp->nwriters--;

        if (--op->refCount == 0){        // last reader
            if (pp->nreaders <= 0){         // no more writers
                pp->busy = 0;             // free the pipe
                return;
            }
        }
        kwakeup(&pp->data);
        return;
    }
}