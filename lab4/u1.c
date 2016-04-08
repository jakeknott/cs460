/*******************************  u1.c ****************************/

#include "ucode.c"
int color;
main0(char *s)
{
    //printf("in main0: 1: recived string = %s\n", s);
    main();
}
main()
{ 
    char name[64]; int pid, cmd, i;

    //set_fg(1);

    printf("In uMode\n");

    while(1)
    {
        pid = getpid();
        color = (pid % 16) + 1;
       
        printf("--------------------- u1 code -------------------------\n");
        printf("I am proc %d in U mode: running segment=\n",getpid(), getcs());
        /*for(i = 0; i < 10000000; i++)
        {

        }*/
        show_menu();
        printf("Command ? ");
        gets(name);
        printf("\nName = %s\n", name);



        //clear_time();
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
            case 15: tsleep(); break;
            case 16: sin(); break;
            case 17: sout(); break;

            default: invalid(name); break;
        }
    }
}
