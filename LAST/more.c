#include "ucode.c"

main(int argc, char *argv[])
{
    char c, readbuf[1024];
    STAT *s;
    int size, fd, i, initalprint = 0, sfd;
    int inre = 0, outre = 0;
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
        printf("*        Jake Knott's More       *\n");
        printf("==================================\n");
    }



    if(argc == 1)
    {
        if(inre)
        {
            fstat(0, s); //stat stdin
            size = s->st_size;
            while(1)//still lines to read
            {
                for(i = 0; i < 1024; i++)
                    readbuf[i] = '\0';

                i = 0;
                while (1) //read one line
                {
                    readbuf[i] = getc();
                    if(c == 255) //end of input
                        exit(0);
                    if (readbuf[i] == '\n' || i > 1024)
                        break;
                    size--;
                    i++;
                }

                readbuf[++i] = '\0';
                initalprint++;
                if(initalprint > 20)
                {
                    do
                    {
                        gettty(tty);
                        fd = open(tty, O_RDONLY);
                        read(fd,&c, 1);
                        close(fd);
                    }
                    while(c != '\r' && c != '\n' && c != 'q' && c != 'd');

                    if(c == 'q')
                        exit(0);

                    if(c == 'd')
                        initalprint = 0;

                    printf("%s", readbuf);//print line
                    printf("\r");
                }
                else
                {
                    printf("%s", readbuf);//print line
                    printf("\r");

                }
            }
        }

    }
    else
    {
        stat(argv[1], s);
        size = s->st_size;
        if(size == 0)
        {
            printf("Error: %s doesn't exist\n", argv[1]);
            return;
        }
        fd = open(argv[1], O_RDONLY);
        while(size > 0)//still lines to read
        {
            for(i = 0; i < 1024; i++)
                readbuf[i] = '\0';

            i = 0;
            while (1) //read one line
            {
                read(fd, &readbuf[i], 1); //read one char at a time place at end
                if (readbuf[i] == '\n' || i > 1024)
                    break;
                size--;
                i++;
            }

            readbuf[++i] = '\0';
            initalprint++;
            if(initalprint > 20)
            {
                do
                    c = getc();
                while(c != '\r' && c != '\n' && c != 'q' && c != 'd');

                if(c == 'q')
                {
                    close(fd);
                    exit(0);
                }
                if(c == 'd')
                    initalprint = 0;

                printf("%s", readbuf);//print line
                printf("\r");
            }
            else
            {
                printf("%s", readbuf);//print line
                printf("\r");
            }
        }
        close(fd);
    }
    exit(0);
}