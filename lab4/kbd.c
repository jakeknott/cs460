#define KEYBD	         0x60	/* I/O port for keyboard data */
#define PORT_B           0x61   /* port_B of 8255 */
#define KBIT	         0x80	/* bit used to ack characters to keyboard */

#define NSCAN             64	/* Number of scan codes */
#define KBLEN             64    // input buffer size

#define BELL 0x07
#define F1 0x3B // scan code of function keys
#define F2 0x3C
#define F3 0x3D
#define F4 0x3E
#define F12 0x58
#define CAPSLOCK 0x3A // scan code of special keys
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define CONTROL 0x1D
#define ALT 0x38
#define DEL 0x53

char unshift[NSCAN] = {
        0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
        'q','w','e','r','t','y','u','i',      'o','p','[',']', '\r', 0,'a','s',
        'd','f','g','h','j','k','l',';',       0, 0, 0, 0,'z','x','c','v',
        'b','n','m',',','.','/', 0,'*',        0, ' '
};

/* Scan codes to ASCII for shifted keys; unused keys are left out */
char shift[NSCAN] = {
        0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
        'Q','W','E','R','T','Y','U','I',      'O','P','{','}', '\r', 0,'A','S',
        'D','F','G','H','J','K','L',':',       0,'~', 0,'|','Z','X','C','V',
        'B','N','M','<','>','?',0,'*',         0, ' '
};


typedef struct kbd{
    char buf[KBLEN];
    int  head, tail;
    SEMAPHORE data;
}KBD;

KBD kbd;

extern int color;

int P(SEMAPHORE *s)
{
    //int sr = int_off();
    int i;
    s->value--;
    if (s->value < 0){
        running->status = BLOCK;
        enqueue(&(s->queue), running);
        tswitch(0);
        int_on();
    }
}

int V(SEMAPHORE *s)
{
    PROC *p;
    int i;
    //int sr = int_off();
    s->value++;
    //printf("value in V = %d\n", s->value);
    if (s->value <= 0){
        p = dequeue(&s->queue, 0);

        p->status = READY;
        enqueue(&readyQueue, p);

    }
    //int_on(sr);
}

int kbd_init()
{
    printf("kbinit()\n");

    kbd.head = 0;
    kbd.tail = 0;

    // kb.data.value = kb.data.queue = 0;
    kbd.data.value = 0;
    kbd.data.queue = 0;

    out_byte(0X20, 0X20);
    printf("kbinit done\n");
}

int alt, capslock, esc, shifted, control, arrowKey;

int kbhandler()
{
    int scode, value, c;
    KBD *kp = &kbd;

    scode = in_byte(KEYBD); // get scan code
    value = in_byte(PORT_B); // strobe PORT_B to ack the char
    out_byte(PORT_B, value | KBIT);
    out_byte(PORT_B, value);

    if (scode & 0x80)
    {// key release: ONLY catch shift
        scode &= 0x7F; // mask out bit 7

        if (scode == LSHIFT || scode == RSHIFT)
            shifted = 0; // released the shift key
        goto out;
    }

    // from here on, must be key press
    if (scode == 1) // Esc key on keyboard
        goto out;
    if (scode == LSHIFT || scode == RSHIFT)
    {
        shifted = 1; // set shifted flag
        goto out;
    }
    if (scode == 0x3A)
    {
        capslock++; // capslock key acts like a toggle
        capslock %= 2;
        goto out;
    }


/************* Catch and handle F keys for debugging *************/
    if (scode == F1)
    {
        kps();
        goto out;
    }
    if (scode == F2)
    {
        printList("readyQueue", readyQueue);
        goto out;
    }
    if (scode == F3)
    {
        printList("freelist", freeList);
        goto out;
    }
    if( scode == F4)
    {
        printSleep();
        goto out;
    }
    if (scode == F12)
    {
        kexit(F12);
        goto out;
    }

    // translate scan code to ASCII, using shift[ ] table if shifted;
    if(shifted)
        c = shift[scode];
    else
        c = unshift[scode];

    // Convert all to upper case if capslock is on
    if (capslock){
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
    }

    /* enter the char in kbd.buf[ ] for process to get */
    if (kbd.data.value == KBLEN)
    {
        printf("%c\n", BELL);
        goto out; // kb.buf[] already FULL
    }
    kbd.buf[kbd.head++] = c;
    kbd.head %= KBLEN;

    V(&kbd.data);
    out:
    out_byte(0x20, 0x20);
}

/********************** upper half rotuine ***********************/

int getc()
{
    int c;
    P(&kbd.data);

    lock();
        c = kbd.buf[kbd.tail++] & 0x7F;
        kbd.tail %= KBLEN;
    unlock();

    return c;
}