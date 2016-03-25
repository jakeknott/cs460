typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

#define READ_PIPE  4
#define WRITE_PIPE 5
#define BROKEN_PIPE 99

#define NOFT      20
#define NFD       10

/*********** timer ****************/
#define SQUARE_WAVE 0x36
#define TIMER_FREQ 1193182L
#define TIMER_COUNT (TIMER_FREQ/60)
#define TIMER_PORT 0x40
#define TIMER_MODE 0x43
#define TIMER_IRQ 0
#define INT_CNTL 0x20
#define INT_MASK 0x21
int tick = 0;


typedef struct Oft{
    int   mode;
    int   refCount;
    struct pipe *pipe_ptr;
} OFT;

#define PSIZE 10
#define NPIPE 10

typedef struct pipe{
    char  buf[PSIZE];
    int   head, tail, data, room;
    int   nreaders, nwriters;
    int   busy;
}PIPE;

typedef struct proc{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8
    int    time;

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];           // name string of PROC

    OFT    *fd[NFD];

    int    kstack[SSIZE];      // per proc stack area
}PROC;

typedef struct header{
    u32 ID_space;         // 0x04100301:combined I&D or 0x04200301:separate I&D
    u32 magic_number;     // 0x00000020
    u32 tsize;            // code section size in bytes
    u32 dsize;            // initialized data section size in bytes
    u32 bsize;            // bss section size in bytes
    u32 zero;             // 0
    u32 total_size;       // total memory size, including heap
    u32 symbolTable_size; // only if symbol table is present
}HEADER;

//used for ext2 file system
typedef struct ext2_inode {
    u16	i_mode;		/* File mode */
    u16	i_uid;		/* Owner Uid */
    u32	i_size;		/* Size in bytes */
    u32	i_atime;	/* Access time */
    u32	i_ctime;	/* Creation time */
    u32	i_mtime;	/* Modification time */
    u32	i_dtime;	/* Deletion Time */
    u16	i_gid;		/* Group Id */
    u16	i_links_count;	/* Links count */
    u32	i_blocks;	/* Blocks count */
    u32	i_flags;	/* File flags */
    u32     dummy;
    u32	i_block[15];    /* Pointers to blocks */
    u32     pad[7];         /* inode size MUST be 128 bytes */
} INODE;

typedef struct ext2_group_desc
{
    u32	bg_block_bitmap;		/* Blocks bitmap block */
    u32	bg_inode_bitmap;		/* Inodes bitmap block */
    u32	bg_inode_table;		/* Inodes table block */
    u16	bg_free_blocks_count;	/* Free blocks count */
    u16	bg_free_inodes_count;	/* Free inodes count */
    u16	bg_used_dirs_count;	/* Directories count */
    u16	bg_pad;
    u32	bg_reserved[3];
} GD;

typedef struct ext2_dir_entry_2 {
    u32	inode;			/* Inode number */
    u16	rec_len;		/* Directory entry length */
    u8	name_len;		/* Name length */
    u8	file_type;
    char	name[255];      	/* File name */
} DIR;