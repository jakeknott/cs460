#include "ucode.c"
int in, out, err;
main(int argc, char *argv[]) {
    char cmd_line[256], *args[64], cmdcp[256], *temp;
    int i = 1, pid, status;
    while (1)
    {
        printf("JRKSH@ ");
        gets(cmd_line);

        strcpy(cmdcp, cmd_line); //saving cmd_line
        args[0] = strtok(cmdcp, " ");
        i = 1;
        while(args[i] = strtok(0, " ")){ i++; }

        //build in commands
        if (strcmp(args[0], "cd") == 0)
        {
            chdir(args[1]);
            continue;
        }
        else if (strcmp(args[0], "logout") == 0 || strcmp(args[0], "exit") == 0)
        {
            exit(0);
            continue;
        }
        else if (strcmp(args[0], "su") == 0)
        {
            setuid(0,0);
            continue;
        }

        // for binary executable command
        pid = fork(); // fork a child sh process
        if (pid) // parent sh
        {
            //this is wrong logic: FIX
            strcpy(cmdcp, cmd_line); //saving cmd_line
            temp = strtok(cmdcp, "&");
            if (strcmp(temp, cmd_line) == 0) // assume at most one & for main sh
            {
                pid = wait(&status);
            }
            continue; // main sh does not wait if &
        }
        else // child sh
            do_pipe(cmd_line, 0);
    }
}

int scan(char *cmdLine, char *head, char *tail)
{
    // divide cmdLine into head and tail by rightmost | symbol
    // cmdLine = cmd1 | cmd2 |...|cmdn-1 | cmdn
    //          |<------- head --------->| tail |; return 1;
    // cmdLine = cmd1 ==> head=cmd1, tail=null; return 0;

    int cmdlength, i, j;

    cmdlength = strlen(cmdLine);

    for(i = cmdlength; i >= 0; i--)
    {
        if (cmdLine[i] == '|')
        {
            break; //i is the right most '|'
        }
    }

    if (i == -1) //no |
    {
        for(i = 0; i < cmdlength; i++)
            head[i] = cmdLine[i];
        head[cmdlength] = 0;
        tail[0] = '\0';
        return 0; //has no pipe
    }
    //else there is a |
    strncpy(head, cmdLine, i); //head = upto right most |
    head[i] = 0;
    for (j = 0; j < cmdlength; j++)
        tail[j] = cmdLine[j+i+2]; //tail = from | on

    return 1; //has pipe
}

int do_pipe(char *cmdLine, int *pd)
{
    int hasPipe, pid, lpd[2];
    char head[64], tail[64];

    if (pd)  // if has a pipe passed in, as WRITER on pipe pd:
    {
        close(pd[0]);
        dup2(pd[1], 1);
        close(pd[1]);
    }
    // divide cmdLine into head, tail by rightmost pipe symbol

    hasPipe = scan(cmdLine, head, tail);
    if (hasPipe)
    {
        //create a pipe
        pipe(lpd);
        pid = fork();
        if (pid) { // parent // reader
            close(lpd[1]);
            dup2(lpd[0], 0);
            close(lpd[0]);
            do_command(tail);
        }
        else
            do_pipe(head, lpd);
    }
    else
        do_command(cmdLine);
}

int do_command(char *cmdLine)
{
    int i = 0, j = 0, fd;
    char head[100], *tokedcmd[100], *s;
    STAT *sty;

    strcpy(head, cmdLine);

    s = strtok(cmdLine, " ");
    tokedcmd[0] = s;
    i = 1;
    while(s = strtok(0, " "))
    {
        tokedcmd[i] = s;
        i++;
    }

    for (i = 0; tokedcmd[i]; i++)
    {
        j += strlen(tokedcmd[i]);
        if (strcmp(tokedcmd[i], "<") == 0) //redirect input
        {
            fd = open(tokedcmd[i + 1], O_RDONLY); // opens the file for read only, redirecting the input
            dup2(fd, 0);
            head[j] = 0;
        }
        else if (strcmp(tokedcmd[i], ">") == 0) //redirect outpu
        {
            fd = open(tokedcmd[i + 1], O_WRONLY|O_CREAT); //opens file for writting only, redirecting the output to the file given
            dup2(fd, 1);
            head[j] = 0;
        }
        else if (strcmp(tokedcmd[i], ">>") == 0)//redirect/ append
        {
            fd = open(tokedcmd[i + 1], O_RDWR|O_APPEND|O_CREAT);
            dup2(fd, 1);
            head[j] = 0;
        }

    }

    exec(head);
}
