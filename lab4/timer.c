int enable_irq(u16 irq)
{ out_byte(INT_MASK, in_byte(INT_MASK) & ~(1 << irq)); }

int tick,sec,min,hr;

int timer_init()
{
    tick = 0; sec = 0; min = 0; hr = 0;
    out_byte(TIMER_MODE, SQUARE_WAVE);
    out_byte(TIMER_PORT, TIMER_COUNT);
    out_byte(TIMER_PORT, TIMER_COUNT >> 8);
    enable_irq(TIMER_IRQ);
}


int tinth();
int thandler()
{
    PROC *sl;
    tick++;
    tick %= 60;
    print_time();
    if (tick == 0) //one second has passed
    {
        addsec();
        if(running->inkmode == 0) //not in kmode, in umode
        {
            running->time--;
            if (running->time == 0 && mode == DEMO)
            {
                running->time = 5;
                out_byte(0x20, 0x20);
                do_tswitch();
            }

            sl = sleepList;
            while (sl != 0)
            {
                sl->time--;
                if(sl->time == 0)
                {
                    kwakeup(sl->name);
                }
                sl = sl->next;
            }
        }

    }
    out_byte(0x20, 0x20);
}