TI_GDT          equ 0000000000000_0_00b             ;TI字段,指定段在GDT中 
RPL0            equ 0000000000000_0_00b             ;RPL特权级0
SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
;------------------------------------------------------------------------------
; 2021/3/24
; function: put_char(uint8_t char_ascii)
; input: an ascii char
; output: 
;
; proced:
;        1) backup regs
;        2) get cursor coordinates
;        3) get char from stack
;        4) judge whether it is control char, if not then put, otherwise, go
;           following procedure
;        5) judge whether scrolling screen is needed, if yes then scroll it
;        6) renew cursor coordinates
;        7) recovery regs
;------------------------------------------------------------------------------
global put_char
put_char:
    pushad                  ; push all 32-bits general regs
                            ; seq: eax, ecx, edx, ebx, esp(origin), 
                            ;      ebp, esi, and edi 
    mov eax,SELECTOR_VIDEO
    mov gs,eax               ; set video selector

    
    ;Controller Data Register: address reg 0x3d4, data reg 0x3d5
    ;cursor location high reg: 0xeh
    ;cursor location low reg: 0xfh

    ;get high 8-bits of cursor
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    in al,dx
    mov ah,al

    ;get low 8-bits of cursor
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    in al,dx
    
    mov bx,ax           ;save cursor
    mov ecx,[esp + 36]   ;get char from stack


    cmp cl,0x08    ;if BackSpace then jmp .is_back_space
    jz .is_back_space

    cmp cl,0x0a    ;elseif LineFeed then jmp .is_line_feed
    jz .is_line_feed

    cmp cl,0x0d    ;elseif CarriageReturn then jmp .is_carriage_return
    jz .is_carriage_return

    jmp .is_other       ;else jmp .is_other

    
    xor dx,dx
    mov ax,bx
    mov si,80
    div si              ;row: ax, col: dx

  .is_back_space:
    dec bx
    shl bx,1
    mov byte [gs:bx],0x20
    inc bx
    mov byte [gs:bx],0x0
    shr bx,1

    jmp .set_cursor


  .is_line_feed:
  .is_carriage_return:

    xor dx,dx
    mov ax,bx
    mov si,80
    div si
    sub bx,dx

    add bx,80           ;next line
    cmp bx,2000

    jge .scroll_screen
    jmp .set_cursor


  .is_other:
    shl bx,1
    mov byte [gs:bx],cl
    inc bx
    mov byte [gs:bx],0x07
    inc bx
    shr bx,1
    
    cmp bx,2000
    jge .scroll_screen
    jmp .set_cursor

  .scroll_screen:
    cld
    mov ecx,960             ;2000-80=1920char 1920*2=3840bytes mov dword per times
                            ;so, 3840/4=960
    mov esi,0xc00b80a0      ;80 char per row, 
                            ;but 160 bytes(including 1byte of attribute)
                            ;so, 0xc00b8000+80*2(0x50*2) = 0xc00b80a0
    mov edi,0xc00b8000      
    rep movsd

    mov ebx,3840
    mov ecx,80
    ;clear the bottom line
    .cls:
      mov word [gs:ebx],0x0720
      add ebx,2
      loop .cls

      mov bx,1920           ;current position, bx=row*80+col


  .set_cursor:
    ;set high 8-bits of cursor
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    mov al,bh
    out dx,al

    ;set low 8-bits of cursor
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    mov al,bl
    out dx,al

    popad
	ret

;------------------------------------------------------------------------------
; 2021/3/24
; function: put_str(uint8_t* str)
; input: a string
; output: 
;
; proced:
;        1) backup regs
;        2) get cursor coordinates
;        3) get char from stack
;        4) judge whether it is control char, if not then put, otherwise, go
;           following procedure
;        5) judge whether scrolling screen is needed, if yes then scroll it
;        6) renew cursor coordinates
;        7) recovery regs
;------------------------------------------------------------------------------
global put_str
put_str:
    push ebx
    push ecx
    xor ecx,ecx
    mov ebx,[esp + 12] ;get string pointer from stack
  .proc:
    mov cl,[ebx]
    cmp cl,0x0
    jz .over
    push ecx
    call put_char
    add esp,4          ;clean ecx in stack
    inc ebx
    jmp .proc
  .over:
    pop ecx
    pop ebx
    ret
