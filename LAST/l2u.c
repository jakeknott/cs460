#include "ucode.c"


main(int argc, char *argv[])
{
    char c, readbuf[1024];
    STAT *s;
    int size, fd1, fd2, outre = 0, inre = 0;
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
        printf("*        Jake Knott's l2u        *\n");
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
                    putc(toupper(c));

                size = size - 1;
            }
        }
        else //normial stdin and stdout just echo input
        {
            while (1) {
                c = getc();
                if (c == '`') {
                    exit(0);
                    break;
                }
                else {
                    putc(toupper(c));
                }
            }
        }
    }
    else if (argc == 2)
    {
        printf("Usage: l2u [fromfile tofile]\n");
        exit(-1);
    }
    else // l2u f1 f2
    {
        stat(argv[1], s);
        size = s->st_size;
        if(size == 0)
        {
            printf("Error: %s doesn't exist\n", argv[1]);
            return;
        }
        fd1 = open(argv[1], O_RDONLY);
        fd2 = open(argv[2], O_CREAT|O_WRONLY);
        while(size > 0)
        {
            read(fd1, &c, 1);
            c = toupper(c);
            write(fd2, &c, 1);
            size--;
        }

    }
    exit(0);
}