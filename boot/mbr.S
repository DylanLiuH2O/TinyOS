%include "boot.inc"
;main boot record
;-----------------------------------------
SECTION MBR vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax       ;display memory


;use 0x06 to clear screen
;-----------------------------------------
;INT 0x10   func. number:0x06    func. description:roll up screen
;input:
;AH func-number=0x06
;BH = roll up attribute
;(CL, CH) = left-top position
;(DL, DH) = right-bottom position
;no-return
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0        ;(0, 0)
    mov dx, 0x184f  ;(80, 25)

    int 0x10

;.get_cursor
    mov ah, 3
    mov bh, 0

    int 0x10
;get_cursor end

;put_string
    ;mov ax, message
    ;mov bp, ax

    ;mov cx, 13      ;lengh of message
    ;mov ax, 0x1301

    ;mov bx, 0x2

    ;int 0x10

    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0xA4     ; set display attribute

    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0xA4

    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0xA4

    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0xA4

    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0xA4
;put_sting end

    mov eax,LOADER_START_SECTOR ; the beginning sector address (lba mode)
    mov bx,LOADER_BASE_ADDR     ; the position that the data will be placed
    mov cx,4                    ; quantity of sectors
    call rd_disk_m_16

    jmp LOADER_BASE_ADDR+0x300
;-----------------------------------------
;function: read n sectors to somewhere you assign
;input:
;   eax = LBA sector number
;   bx  = the position that the data will be placed
;   cx  = numbers of sector that have not been read yet
;-----------------------------------------
rd_disk_m_16:
    mov esi,eax     ; push regs'
    mov di,cx
;read hard disk;
;step1
    mov dx,0x1f2
    mov al,cl
    out dx,al

    mov eax,esi
;step2
    ;LBA addr 7~0 --> 0x1f3
    mov dx,0x1f3
    out dx,al

    ;LBA addr 15~8 --> 0x1f4
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    ;LBA addr 23~16 --> 0x1f5
    shr eax,cl
    mov dx,0x1f5
    out dx,al

    ;LBA addr 27~24 --> 0x1f6
    shr eax,cl
    and al,0x0f     ;无关位清零
    or al,0xe0      ;1110, LBA mode
    mov dx,0x1f6
    out dx,al

;step3  wrtie a read command(0x20) to 0x1f7 
    mov dx,0x1f7
    mov al,0x20
    out dx,al

;step4 check status of hard disk 
.not_ready:
    nop
    in al,dx
    and al,0x88    ;保留第4位和第8位
    cmp al,0x08    ;判断状态
    jnz .not_ready

;step5 read from 0x1f0
    ;设置读取参数
    mov ax,di
    mov dx,256
    mul dx      ;每次只能读一个字,即2个字节,那么读一个扇区要256次,读n个扇区要n*256次
    mov cx,ax
    mov dx,0x1f0

.go_on_read:
    in ax,dx
    mov [bx],ax
    add bx,2
    loop .go_on_read

    ret     ; 调用返回

    ;message db "Hello! My MBR"
    times 510-($-$$) db 0
    db 0x55,0xaa

