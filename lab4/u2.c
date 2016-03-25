/*******************************  u2.c ****************************/

#include "ucode.c"
int color;
main0(char *s)
{
    /*char args[100];

    printf("in main0: 2: recived string  = %s\n", s);*/
    main();
}
main()
{
    char name[64]; int pid, cmd;

    printf("-------------------------------------------------\n");
    printf("               In Umode - u2.c running           \n");
    printf("-------------------------------------------------\n");


    while(1)
    {
        pid = getpid();
        color = pid + 5;

        printf("--------------------- u2 code -------------------------\n");
        printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
        show_menu();
        printf("Command ? ");
        gets(name);
        printf("Name = %s\n", name);
        if (name[0]==0)
            continue;

        cmd = find_cmd(name);
        switch(cmd){
            case 0 : getpid();   break;
            case 1 : ps();       break;
            case 2 : chname();   break;
            case 3 : kmode();    break; //kfork(); break;
            case 4 : kswitch();  break;
            case 5 : wait();     break;
            case 6 : exit();     break;
            case 7: getchar(); break;
            case 8: fork(); break;
            case 9: exec(); break;

            case 10:  pipe(); break;
            case 11: pfd();  break;
            case 12: read_pipe(); break;
            case 13: write_pipe(); break;
            case 14: close_pipe(); break;

            default: invalid(name); break;
        }
    }
}
