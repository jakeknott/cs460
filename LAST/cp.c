#include "ucode.c"


main(int argc, char *argv[])
{
    char c, readbuf[1024];
    STAT *s;
    int size, fd1, fd2;

    printf("==================================\n");
    printf("*        Jake Knott's cp         *\n");
    printf("==================================\n");

    if(argc != 3)
    {
        printf("Usage: cp scr dest\n");
        exit(-1);
    }
    else // cp scr dest
    {
        stat(argv[1], s);
        size = s->st_size;
        if(size == 0)
        {
            printf("Error: %s doesn't exist\n", argv[1]);
            return;
        }
        fd1 = open(argv[1], O_RDONLY);
        fd2 = open(argv[2], O_CREAT|O_WRONLY, 0644);
        while(size > 0)
        {
            if(size >= 1024)
            {
                read(fd1, readbuf, 1024);
                write(fd2, readbuf, 1024);
                size-=1024;
            }
            else
            {
                read(fd1, readbuf, size);
                write(fd2, readbuf, size);
                size -= size;
            }

        }

    }
    exit(0);
}