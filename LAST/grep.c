#include "ucode.c"

main(int argc, char *argv[])
{
    char c, readbuf[1024], find[100], *str;
    STAT *s;
    int size, fd, i, j, k = 0, findLen = 0, isquotes = 0;
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
        printf("*        Jake Knott's Grep       *\n");
        printf("==================================\n");
    }


    printf("");

    for (i = 0; i < argc; i++)
    {
        for(j = 0; j < strlen(argv[i]) ; j++)
        {
            if(argv[i][j] == '"')
            {
                isquotes = 1;
                readbuf[k] = '/';
            }
            else
                readbuf[k] = argv[i][j];
            k++;
        }
        readbuf[k] = ' ';
        k++;
    }

    if(isquotes)
    {
        str = strtok(readbuf, "/");
        str = strtok(0, "/"); //find now points to the string between the quotes4

        findLen = strlen(str);
        for (i = 0; i < findLen; i++)
            find[i] = str[i];
        find[findLen] = 0;
    }
    else
        strcpy(find, argv[1]);


    findLen = strlen(find);

    if(argc == 2) //no file, just from stdin
    {
        fstat(0, s); //stat stdin
        while (1)
        {
            for(i = 0; i < 1024; i++) //zero out buff
                readbuf[i] = '\0';

            i = 0;
            while (1) //read one line
            {
                readbuf[i] = getc();
                if(readbuf[i] == 255) //end of input
                    exit(0);
                if (readbuf[i] == '\n' || i > 1024) {
                    break;
                }
                i++;
            }
            // readbuf is one line from file
            //check to see if find is in readbuf
            for (i = 0; i < strlen(readbuf); i++) {
                if (strncmp(&readbuf[i], find, findLen) == 0) {
                    printf("%s\r", readbuf);
                    break;
                }

            }
        }
    }
    else
    {
        stat(argv[argc - 1], s); //stat file to make sure it exist
        size = s->st_size;
        if(size == 0)
        {
            printf("Error: %s doesn't exist\n", argv[1]);
            return;
        }
        fd = open(argv[argc - 1], O_RDONLY); //open for read

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
            // readbuf is one line from file
            //check to see if find is in readbuf
            for (i = 0; i < strlen(readbuf); i++)
            {
                if(strncmp(&readbuf[i], find, findLen) == 0)
                    printf("%s\r", readbuf);
            }
        }
        close(fd);
    }
    exit(0);
}