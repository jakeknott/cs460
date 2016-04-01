        MTXSEG  = 0x1000
	
       .globl _main,_running,_scheduler
       .globl _proc, _procSize
       .globl _tswitch
       .globl _diskr,_setes,_inces,_printf
       .global _tinth, _thandler
       .global _lock, _unlock, _in_byte, _out_byte, _int_on, _int_off
       .global  _kbhandler, _kbinth
	
        jmpi   start,MTXSEG

start:	mov  ax,cs
	mov  ds,ax
	mov  ss,ax
        mov  es,ax
	mov  sp,#_proc
	add  sp,_procSize

	mov ax,#0x0003
	int #0x10
		
	call _main

_tswitch:
        cli
	    push ax
	    push bx
	    push cx
	    push dx
	    push bp
	    push si
	    push di
	    pushf
	    mov  bx,_running
	    mov  2[bx],sp

    FIND:	call _scheduler

    RESUME:
	    mov  bx,_running
	    mov  sp,2[bx]
	    popf
	    pop  di
	    pop  si
	    pop  bp
	    pop  dx
	    pop  cx
	    pop  bx
	    pop  ax

        sti
	    ret

	
! added functions for KUMODE
	.globl _int80h,_goUmode,_kcinth, _ireturn
!These offsets are defined in struct proc
USS =   4
USP =   6
INK = 8

    MACRO INTH ! as86 macro: parameters are ?1 ?2, etc
        push ax
        push bx
        push cx
        push dx
        push bp
        push si
        push di
        push es
        push ds
        mov ax,cs
        mov ds,ax ! Assume one-segment kernel: set DS to kDS=0x1000
        mov bx,_running ! bx points to running PROC
        inc INK[bx] ! enter Kmode : inc proc.inkmode by 1
        cmp INK[bx],#1 ! if proc.inkmode=1,interrupt was in Umode
        jg ?1 ! inkmode > 1 : interrupt was in Kmode
        !---- interrupt in Umode: save interrupted (SS,SP) into PROC -------
        mov bx,_running ! ready to access running PROC
        mov USS[bx],ss ! save SS in PROC.uss
        mov USP[bx],sp ! save SP in PROC.usp
        ! change ES,SS to Kernel segment
        mov ax,ds
        mov es,ax ! CS=DS=SS=ES in Kmode
        mov ss,ax
        mov sp, _running ! sp -> running PROC
        add sp, _procSize ! sp -> running proc's kstack high end
?1:     call _?1 ! call handler in C
        br _ireturn ! return to interrupted point
      MEND



! interrupt handlers; by MACRO INTH(label, handler, parameter)

_int80h: INTH kcinth
_tinth:  INTH thandler
_kbinth: INTH kbhandler

!*===========================================================================*
!*		_ireturn  and  goUmode()       				     *
!*===========================================================================*
! ustack contains    flag,ucs,upc, ax,bx,cx,dx,bp,si,di,es,ds
! uSS and uSP are in proc
_ireturn:                       ! return from INTERRUPTs
_goUmode:                       ! goUmode(): same as return from INTERRUPT
        cli
	    mov bx,_running 	! bx -> proc
        dec INK[bx]
        cmp INK[bx],#0
        jg  xkmode

! return to Umode, restore ustack (uSS, uSP)m then pop ustack
        mov ax,USS[bx]
        mov ss,ax               ! restore SS
        mov sp,USP[bx]          ! restore SP
xkmode:
	    pop ds
	    pop es
	    pop di
        pop si
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax
        iret

                               !        4    6      8     10
_diskr:                        ! diskr(cyl, head, sector, buf)
        push  bp
	mov   bp,sp

        movb  dl, #0x00        ! drive 0=fd0 in DL
        movb  dh, 6[bp]        ! head        in DH
        movb  cl, 8[bp]        ! sector      in CL
        incb  cl               ! inc sector by 1 to suit BIOS
        movb  ch, 4[bp]        ! cyl         in CH
        mov   ax, #0x0202      ! (AH)=0x02=READ, (AL)=02 sectors
        mov   bx, 10[bp]       ! put buf value in BX ==> addr = [ES,BX]

!---------- call BIOS INT 0x13 ------------------------------------
        int  0x13              ! call BIOS to read the block
!-----------------------------------------------------------------
        jb   error             ! to error if CarryBit is on [read failed]

	mov   sp,bp
	pop   bp
	ret

_setes:  push  bp              ! setes(segment): set ES to a segment
	 mov   bp,sp

         mov   ax,4[bp]
         mov   es,ax

	 mov   sp,bp
	 pop   bp
	 ret

_inces:                        ! inces() inc ES by 0x40, or 1K
         mov   ax,es
         add   ax,#0x40
         mov   es,ax
         ret

error:
        push #msg
        call _printf
        int  0x19                       ! reboot
msg:    .asciz  "Loading Error!"

!========== lock =======!

_lock:
	cli			! disable interrupts
	ret			! return to caller

!*===========================================================================*
!*				unlock					     *
!*===========================================================================*
_unlock:
	sti			! enable interrupts
	ret			! return to caller

!*===========================================================================*
!*				out_byte				     *
!*==============================================================
! out_byte[port_t port, int value];
! Write  value  [cast to a byte]  to the I/O port  port.

_out_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
        mov     ax,6[bp]
	    outb	dx,al   	! output 1 byte
        pop     bp
        ret

!*===========================================================================*
!*				in_byte					     *
!*===========================================================================*
! PUBLIC unsigned in_byte[port_t port];
! Read an [unsigned] byte from the i/o port  port  and return it.

_in_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
	inb     al,dx		! input 1 byte
	subb	ah,ah		! unsign extend
        pop     bp
        ret

_int_off:             ! cli, return old flag register
        pushf
        cli
        pop ax
        ret

_int_on:              ! int_on(int SR)
        push bp
        mov  bp,sp
        mov  ax,4[bp] ! get SR passed in
        push ax
        popf
        pop  bp
        ret
