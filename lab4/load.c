/*******************************  load.c ****************************/

#define BLOCK_SIZE 1024

INODE *ip;
char buf2[BLOCK_SIZE];

int get_block(u16 blk, char *buff)  // load disk block blk to char buf[1024]
{
    // Convert blk into (C,H,S) format by Mailman to suit disk geometry
    //      CYL         HEAD            SECTOR
    diskr( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buff);
}

u16 search(INODE *ip, char* name)
{
    int i;
    char c;
    DIR *dp;

    for(i = 0; i < 12; i++)
    {
        if ((u16)ip->i_block[0])
        {
            get_block((u16)ip->i_block[i], buf2);
            dp = (DIR *) buf2;

            while((char *) dp < &buf2[BLOCK_SIZE])
            {
                c = dp->name[dp->name_len];
                dp->name[dp->name_len] = 0;
                printf("%s  ", dp->name);
                if(strcmp(dp->name, name) == 0)
                {
                    printf("\nfound %s\n", name);
                    return((u16)dp->inode);
                }
                dp->name[dp->name_len] = c;
                dp = (char *) dp + dp->rec_len;
            }
        }
    }
    return -1;//cant fint file name
}

load(char *filename, u16 segment)
{
    HEADER *hp;
    char buf1[BLOCK_SIZE];
    u16 ino, j;
    GD* gp;
    char *name[100], savename[100];
    u16 i = 1, namecount = 1;
    u32 size;
    u16 InoBegin;

    setes(0x1000);
    printf("Loading %s into %x.....\n", filename, segment);

    //getting root ino
    get_block(2, buf1);
    gp = (GD *)buf1;


    strcpy(savename, filename); //saving filename
    //splitting filename into srings by '/'
    name[0] = strtok(filename, "/");
    while (name[i] = strtok(0, "/"))
    {
        i++;
        namecount++;
    }


    InoBegin = (u16)gp->bg_inode_table; //inode begin block
    get_block(InoBegin, buf1); //get inode begin block
    ip =(INODE *)buf1+1; //ip->root is inode 2

    for(i = 0; i < namecount; i++)
    {
        ino = search(ip, name[i]) - 1;

        printf("ino: %d\n", ino);
        if (ino < 0)
        {
            printf("Load Failed: Can't find %s\n", filename);
            return -1; //return failed
        }
        //mailmans algorithm to find the inode
        get_block(InoBegin + (ino / 8) , buf1);
        ip = (INODE *)buf1 + (ino % 8);
    }
    printf("found %s at %d\n", savename, ino);

    //loading header
    get_block((u16)ip->i_block[0], buf2);
    hp = (HEADER *)buf2;

    size = hp->tsize + hp->dsize;

    get_block(InoBegin + (ino / 8) , buf1);
    ip = (INODE *)buf1 + (ino % 8);

    setes(segment);//change ES to segment to load to segment, use ip (global)
    {
        for(i = 0; i <= (size / 1024) + 1 ; i++)
        {
            get_block((u16)ip->i_block[i], 0);
            inces(); //move es up 1k
        }
        //must move to chop off head
        for(i = 0; i < size; i++)
        {
            put_byte(get_byte(segment , i + 32), segment, i); //get byte from i+32, place that byte at i
        }
    }

    get_block((u16)ip->i_block[0], buf1);
    hp = (HEADER *)buf1;
    for(i = 0; i < hp->bsize; i++)
    {
        put_byte(0, segment, i + size);
    }
    hp->bsize = 0;

    strcpy(filename, savename);

    printf("......done\n");
    return 1;
}