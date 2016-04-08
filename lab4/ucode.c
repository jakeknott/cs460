/*******************************  ucode.c ****************************/


char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", "getchar", "fork", "exec", "pipe", "pfd", "read", "write", "close", "sleep", "sin", "sout", 0};

#define LEN 64

void putc(char c)
{
    syscall(8, c);
}

char getc()
{
    char c;
    c = syscall(7,0,0,0);
    return c;
}

char* gets(char in[64])
{
    char c;
    int i = 0;
    for(i = 0; i < 64; i++)
    {
        in[i] = 0;
    }
    i = 0;
    while((c = getc()) != '\r')
    {
        putc(c);
        in[i] = c;
        i++;
    }
    in[i] = 0;
    return in;

}


int show_menu()
{
    printf("***************** Menu ***************************************\n");
    printf("* getpid ps  chname  kmode  switch  wait fork exec           *\n");
    printf("* sleep  pipe pfd read write close exit sin sout             *\n");
    printf("**************************************************************\n");
}

int find_cmd(char *name)
{
    int i = 0;
    for(i = 0 ; i <= 17; i++)
    {
        if (strcmp(name,cmd[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kfork()
{
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid);
  child = syscall(3, 0, 0);
  printf("proc %d kforked a child %d\n", pid);
}    

int kswitch()
{
    return syscall(4,2,0); //'2' tells the kernel to change running procs->status to READY, from RUNNING
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n");
} 

int geti()
{
    char *s;
    int i;
    gets(s);
    i = atoi(s);
}

int exit()
{
   int exitValue;
   printf("enter an exitValue: ");
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

int getchar()
{
    char c;
    printf("enter char: ");
    c = syscall(7,0,0);
    syscall(8,c,0);
    printf("\n");
}

int fork()
{
    int pid;
    printf("Enter kernel to fork...\n");
    pid = syscall(9,0,0,0);
    if(pid == -1)
        return -1;

    if(pid == 0)
    {
        printf("I am child returned pid = %d\n", pid);
    }
    else
    {
        printf("I am parent returned pid = %d\n", pid);
    }
    return 0;
}

int exec()
{
    char s[64];
    printf("Enter filename: ");
    gets(s);
    printf("\n");
    return syscall(10,s,0,0);
}

int kmode()
{
    syscall(11,0,0,0);
}

int pd[2];

int pipe()
{
   printf("pipe syscall\n");
   syscall(30, pd, 0);
   printf("proc %d created a pipe with fd = %d %d\n", getpid(), pd[0], pd[1]);
}

int pfd()
{
  syscall(34,0,0,0);
}
  
int read_pipe()
{
  char fds[32], buf[1024]; 
  int fd, n, nbytes;
  pfd();

  printf("read : enter fd nbytes : ");
  gets(fds);
  sscanf(fds, "%d %d",&fd, &nbytes);
  printf("fd=%d  nbytes=%d\n", fd, nbytes); 

  n = syscall(31, fd, buf, nbytes);

  if (n>=0){
     printf("proc %d back to Umode, read %d bytes from pipe : ",
             getpid(), n);
     buf[n]=0;
     printf("%s\n", buf);
  }
  else
    printf("read pipe failed\n");
}

int write_pipe()
{
  char fds[16], buf[1024]; 
  int fd, n, nbytes;
  pfd();
  printf("write : enter fd text : ");
  gets(fds);
  sscanf(fds, "%d %s", &fd, buf);
  nbytes = strlen(buf);
            
  printf("fd=%d nbytes=%d : %s\n", fd, nbytes, buf);

  n = syscall(32,fd,buf,nbytes);

  if (n>=0){
     printf("\nproc %d back to Umode, wrote %d bytes to pipe\n", getpid(),n);
  }
  else
    printf("write pipe failed\n");
}

int close_pipe()
{
  char s[16]; 
  int fd;
    pfd();
  printf("enter fd to close : ");
  gets(s);
  fd = atoi(s);
  syscall(33, fd);
}



void clear_time()
{
    syscall(98, 0);
}

void tsleep()
{
    char t[8];
    printf("Enter time to sleep: ");
    gets(t);

    syscall(12, t);
    printf("*******************************************\n");
    printf("*             BACK FROM SLEEP             *\n");
    printf("*******************************************\n");

}

int sout()
{
    char port;
    char line[64];
    int i = 0;
    for (i = 0 ; i < 64; i++)
    {
        line[i] = '\0';
    }
    printf("input port number:[0|1]");

    port = getc()&0x7F - '0';
    putc(port+'0'); getc();
    printf("\nport = %d\n", port);
    printf("Enter message: ");
    gets(line);
    printf("recived = %s\n", line);
    syscall(14, port, line);
}


int sin()
{
    char port;
    int i = 0;
    char uline[64];
    for (i = 0 ; i < 64; i++)
    {
        uline[i] = '\0';
    }

    printf("sin");
    port = '0';

    syscall(13, port, uline, 0);
    printf("**********  Back in Umode ************\n");
    printf("Line back from Kernal = %s\n", uline);
}