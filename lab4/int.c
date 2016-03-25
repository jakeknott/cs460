/*******************************  int.c ****************************/


/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
   int a,b,c,d, r;
   u16 segment, offset;
   segment = running->uss; offset = running->usp;

   a = get_word(segment, offset + 2*13);
   b = get_word(segment, offset + 2*(13+1));
   c = get_word(segment, offset + 2*(13+2));
   d = get_word(segment, offset + 2*(13+3));

  //==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack
    //
    running->inkmode = 1; //syscalls go to kernel

   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kkfork();         break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;
       case 7: r = getc(); break;
       case 8: r = kputc(b); break;
       case 9: r = ufork(); break;
       case 10: r = uexec(b); break;
       case 11: kmode(); break;
       case 12: tsleep(b); break;

           //Pipe functions
       case 30 : r = kpipe(b); break;
       case 31 : r = read_pipe(b,c,d);  break;
       case 32 : r = write_pipe(b,c,d); break;
       case 33 : r = close_pipe(b);     break;
       case 34 : r = pfd();             break;

       case 98: timeclear(); break;
       case 99: kkexit(b);            break;
       default: printf("invalid syscall # : %d\n", a); 
   }

    running->inkmode = 0; //returning back to umode

   //==> WRITE CODE to let r be the return value to Umode
   put_word(r, segment, offset + 2*8);
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
    return (running->pid);
}

int kps()
{
    int i;
    char *status;
    printf("=======================================================================\n");
    printf("name \t\t status \t" "pid \t ppid\n");
    printf("-----------------------------------------------------------------------\n");

    for (i = 0; i < NPROC; i++)
    {
        switch(proc[i].status)
        {
            case 0 : status = "FREE"; break;
            case 1 : status = "READY"; break;
            case 2 : status = "RUNNING"; break;
            case 3 : status = "STOPPED"; break;
            case 4 : status = "SLEEP"; break;
            case 5 : status = "ZOMBIE"; break;
        }
        printf("%s\t\t" "%s" "\t\t%d \t %d\n", proc[i].name, status, proc[i].pid, proc[i].ppid);
    }
    printf("-----------------------------------------------------------------------\n");
    printf("-----------------------------------------------------------------------\n");

    return 0;
}

int kchname(char *name)
{
    char c;
    int i = 0;
    for (i= 0 ; i < 32; i++)
    {
        c = get_byte(running->uss, name + i); //gets bite from running
        running->name[i] = c;
        if (c == '\0')
            break;
    }
    return 0;
}

int kkfork()
{
  kfork("/bin/u1");
  //use you kfork() in kernel;
  //return child pid or -1 to Umode!!!
}

int ktswitch()
{
    return tswitch();
}

int kkwait(int *status)
{
    int ret, code;
    ret = kwait(&code); //code is the exitcode
    put_word(code, running->uss, status); //write the exitcode to the runnings user stack space (userMode)
    //running->status =
    return(ret); //return the pid of child when killed
  //use YOUR kwait()) in LAB3;
  //return values to Umode!!!
}

int kkexit(int value)
{
  return (kexit(value));
  //use your kexit() in LAB3
  // do NOT let P1 die
}

int ufork()
{
    return fork();
}

int uexec(char *s)
{
    int i = kexec(s);
    return i;
}

int kmode()
{
    body();
}

kputc(char c)
{
    putc(c);
}

int tsleep(char *time)
{
    char c, s[8];
    int i = 0, stime;
    for (i= 0 ; i < 8; i++)
    {
        c = get_byte(running->uss, time + i); //gets byte from running
        s[i] = c;
        if (c == '\0')
            break;
    }

    if(running->pid <= 1)
    {
        printf("Error:Cant sleep on proc 1\n");
        return -1;
    }
    stime = atoi(s);
    printf("Going to sleep for %d seconds\n", stime);
    running->time = stime;
    ksleep(running->name);
}

