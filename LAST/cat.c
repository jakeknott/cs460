#include "ucode.c"

main(int argc, char *argv[])
{
    char c;
    STAT *s;
    int size, fd, inre = 0, outre = 0, r = 0;
    int stty_ino = 0, stty_dev = 0, st0_ino = 0, st0_dev = 0, st1_ino = 0, st1_dev = 0;
    char *tty;

    fstat(1, s);
    st1_ino = s->st_ino; st1_dev = s->st_dev;

    fstat(0, s);
    st0_ino = s->st_ino; st0_dev = s->st_dev;

    gettty(tty);
    stat(tty, s);
    stty_ino = s->st_ino; stty_dev = s->st_dev;

    /*printf("********* st0 *********\n");
    printf("ino: %d\tdev: %d\n", st0_ino, st0_dev);
    printf("********* st1 *********\n");
    printf("ino: %d\tdev: %d\n", st1_ino, st1_dev);
    printf("********* st_tty *********\n");
    printf("ino: %d\tdev: %d\n", stty_ino, stty_dev);
     */

    if(st1_ino!= stty_ino || st1_dev != stty_dev)
    {
        outre = 1;
    }
    if (st0_ino != stty_ino || st0_dev != stty_dev)
    {
        inre = 1;
    }
    if(!outre && !inre)
    {
        printf("==================================\n");
        printf("*        Jake Knott's cat        *\n");
        printf("==================================\n");
    }


    if(argc == 1)
    {
        if(inre)
        {
            fstat(0, s); //stat stdin
            size = s->st_size;
            while (1)
            {
                c = getc();
                if(c == 255) //end of input
                    exit(0);
                if(c != '\r')
                    printf("%c",c);

                size = size - 1;
            }
        }
        else //normial stdin and stdout just echo input
        {
            if(outre)
                fd = open(tty, O_WRONLY);

            while (1)
            {
                c = getc();
                if (c == 27 || c == '`')
                {
                    exit(0);
                    break;
                }
                else
                {
                    gettty(tty);
                    if(outre) // if out redirecte must also write to terminal
                    {
                        if(strcmp(tty, "/dev/tty0") == 0)
                        {
                            if (c == '\r')
                                write(fd, "\n", 1);
                        }
                        write(fd, &c, 1);
                    }

                    if(strcmp(tty, "/dev/tty0") == 0)
                    {
                        if (c == '\r')
                            printf("\n");
                    }
                    printf("%c",c);
                }
            }

            if(outre)
                close(fd);
        }
    }
    else
    {
        stat(argv[1], s);
        size = s->st_size;
        if(size == 0)
        {
            printf("Error: %s doesn't exist\n", argv[1]);
            exit(0);
        }
        fd = open(argv[1], O_RDONLY);
        while (size > 0)
        {
            read(fd, &c, 1);
            if(!outre)
            {
                if (c == '\n')
                    printf("\r");
            }
            printf("%c",c);


            size = size - 1;
        }
        close(fd);
    }

    exit(0);
}