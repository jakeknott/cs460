#include "ucode.c"
char *tty;
int in, out, err;

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
    int file, file_size, i, uid, gid;
    char login[64], pass[64], passbuf[1024], *tokedpass, *homedir, program[64];
    STAT *s;

    tty = argv[1];

    close(0);
    close(1);
    close(2);

    in = open(tty, 0);
    out = open(tty, 1);
    err = open(tty, 2);

    settty(tty);

    printf("JKLOGIN : open %s as stdin, stdout, stderr\n", tty);

    signal(2,1);

    file = open("/etc/passwd", O_RDONLY);
    stat("/etc/passwd", s);
    file_size = s->st_size;

    while (1)
    {
        printf("login: ");
        gets(login);
        printf("password: ");
        gets(pass);

        file_size = s->st_size;

        lseek(file, 0);
        while(file_size > 0)//still lines to read
        {
            for(i = 0; i < 1024; i++)
                passbuf[i] = '\0';

            i = 0;
            while(1) //read one line
            {
                read(file, &passbuf[i], 1); //read one char at a time place at end
                if (passbuf[i] == '\n' || i > 1024)
                    break;
                file_size--;
                i++;
            }
            passbuf[i] = '\0';
            //printf("%s\n", passbuf);
            if (passbuf[0] == '\0')
                break;

            tokedpass = strtok(passbuf, ":");

            if(strcmp(tokedpass, login) == 0)
            {
                tokedpass = strtok(0, ":");
                if(strcmp(tokedpass, pass) == 0)
                {
                    tokedpass = strtok(0, ":"); //gid
                    gid = atoi(tokedpass);
                    tokedpass = strtok(0, ":"); //uid
                    uid = atoi(tokedpass);
                    printf("gid = %d\tuid = %d\n", gid, uid);
                    chuid(uid,gid); //setting uid and gid
                    tokedpass = strtok(0, ":"); //full name
                    printf("==================================\n");
                    printf("*     Welcome %s   *\n", tokedpass);
                    printf("==================================\n");

                    homedir = strtok(0, ":"); //homedir
                    printf("homedir = %s\n", homedir);

                    chdir(homedir);
                    close(file);
                    exec("/bin/sh");
                }
            }
        }
        printf("login failed, try again\n");
    }
}
