int pid, tty0_child, ttyS0_child, ttyS1_child, status;
int in, out;

#include "ucode.c"

int main(int argc, char *argv[])
{
    printf("in initi\n");
    in  = open("/dev/tty0", 0); 	// stdin is READ
    out = open("/dev/tty0", 1);

    // printf() now possible
    printf("#########################################\n");
    printf("JKINIT : fork a login task on console \n");
    printf("#########################################\n");

    tty0_child = fork();

    if (tty0_child)
    {
        ttyS0_child = fork();
        if (ttyS0_child)
        {
            ttyS1_child = fork();
            if (ttyS1_child)
            {
                parent();
            }
            else
            {
                loginS1();
            }
        }
        else
        {
            loginS0();
        }
    }
    else
    {
        login();
    }
}

int login()
{
    exec("login /dev/tty0");
}

int loginS0()
{
    exec("login /dev/ttyS0");
}

int loginS1()
{
    exec("login /dev/ttyS1");
}

int parent()
{
    while(1)
    {
        printf("JKINIT : waiting...\n");

        pid = wait(&status);

        if ( pid == tty0_child || pid == ttyS0_child || pid == ttyS1_child)
        {
            printf("INIT: forking another login");

            if (pid == tty0_child)
            {
                tty0_child = fork();
                if(!tty0_child)
                    login();

            }
            else if (pid == ttyS0_child)
            {
                ttyS0_child = fork();
                if (!ttyS0_child)
                    loginS0();

            }
            else
            {
                ttyS1_child = fork();
                if (!ttyS1_child)
                    loginS1();
            }

        }
        else
        {
            printf("JKINIT: buried an orphan child pid=%d", pid);
        }

    }
}
