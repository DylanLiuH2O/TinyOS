%include "boot.inc"
SECTION LOADER vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR 

    ;jmp loader_start    ;跳到开始  去掉跳转,改为在mrb.S中直接跳转到loader_start,
    ;跨过数据区,数据区恰好512+256字节,偏移为0x300

;-------------------------------------------------------------------------------
; 构建GDT及其内部描述符
;-------------------------------------------------------------------------------
    ;注意数据的顺序,x86是小端(低尾端,低字节在低地址上)存储,代码从上到下地址变大
    GDT_BASE:         dd 0x00000000         ;低32位
                      dd 0x00000000         ;高32位
    CODE_DESC:        dd 0x0000ffff         ;低32位,由于粒度为4KB,且平坦模型下一
                                            ;个段最大寻址范围为4GB,那么段界限值则
                                            ;为4GB/4K=1MB=0x000fffff,这里填低16位

                      dd DESC_CODE_HIGH_4B  ;高32位,直接取boot.inc中定义好的宏
    DATA_STACK_DESC:  dd 0x0000ffff         ;同CDOE_DESC
                      dd DESC_DATA_HIGH_4B  ;同CDOE_DESC
    VIDEO_DESC:       dd 0x80000007         ;显存段不使用平坦模型,起始地址为
                                            ;0x000b8000,分开三部分存储,
                                            ;0x00,0x0b,0x8000,这里高16位存的第1部
                                            ;分0x8000
                                            ;显存区范围0xb8000~0xbffff,段大小
                                            ;0xbffff-0xb8000=0x7fff,根据粒度4KB可
                                            ;算得段界限0x7fff/4K=0x7,填入这里的低
                                            ;16位
                      dd DESC_VIDEO_HIGH_4B

    GDT_SIZE    equ ($ - GDT_BASE)          ;$为当前地址,减去GDT_BASE得到GDT大小
    GDT_LIMIT   equ GDT_SIZE-1

    times 60 dq 0                           ;double-quad-word,8个字节,这里预留60
                                            ;个描述符的空位

    total_mem_bytes dd 0                    ;用于保存内存容量,单位是字节
                                            ;这里相对于文件头的偏移地址为0x200,
                                            ;前面刚好512字节
                                            ;而loader加载到0x900,因此这里实际的
                                            ;物理地址为0x900+0x200=0xb00

    ;定义各段的选择子
    SELECTOR_CODE   equ (0x0001 << 3) + TI_GDT + RPL0   ;(0x0001 << 3)是描述符索
                                                        ;引值,其值取决于段描述符
                                                        ;在GDT中的位置
    SELECTOR_DATA   equ (0x0002 << 3) + TI_GDT + RPL0   ;同上
    SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0   ;同上

    gdt_ptr dw GDT_LIMIT                                ;用于lgdt加载gdt,lgdt指令
                                                        ;格式为lgdt 48位内存数据

            dd GDT_BASE                                 ;这48位内存数据,低16位GDT
                                                        ;以字节为大小的偏移量,
                                                        ;高32位为GDT起始地址

    ;loadermsg db '2 loader in real.'

    ards_buf times 244 db 0                             ;total_mem_bytes(4B) + 
                                                        ;gdt_ptr(6B) + 
                                                        ;ards_buf(244B) + 
                                                        ;ard_nr(2B)=256B,对齐

    ards_num dw 0                                       ;记录ards结构体数量
;-------------------------------------------------------------------------------



loader_start:
;-------------------------------------------------------------------------------
; 使用三种方法检测内存容量 
;-------------------------------------------------------------------------------
    ;---------------- 方法1:使用中断0x15子功能0xe820h获取内存布局 ----
    xor ebx,ebx                 ;清零
    mov edx,0x534d4150          ;SMAP的ASCII码作为签名
    mov di,ards_buf             ;ards_buf缓冲区地址
.e820_get_mem_loop:
    mov eax,0x0000e820          ;每次执行中断后,eax的值会变成签名的值,
                                ;所以要每次都更改回来
    mov ecx,20                  ;ards结构字节的大小,指示BIOS写入的字节数
    int 0x15
    jc .e820_failed_jmp_to_e801  ;检测CF位,如果为1则调用出错,跳转到下一个方法执行
    add di,cx
    inc word [ards_num]         ;记录数量
    cmp ebx,0                   ;判断这是否最后一个ards结构
    jnz .e820_get_mem_loop


    mov cx,[ards_num]
    mov ebx,ards_buf
    xor edx,edx                 ;用edx来存储最大值
.find_max_mem_area:
    mov eax,[ebx]
    add eax,[ebx+8]
    add ebx,20
    cmp edx,eax
    jge .next_ards
    mov edx,eax
    .next_ards:
    loop .find_max_mem_area

    ;---------------- 方法2:使用中断0x15子功能0xe810h获取内存信息 ----
.e820_failed_jmp_to_e801:
    mov ax,0xe801
    int 0x15
    jc .e801_failed_jmp_to_88

    mov cx,0x400        ;单位1KB
    mul cx

    ;获得完整的32位积
    shl edx,16
    and eax,0x0000ffff
    or edx,eax

    add edx,0x100000    ;加上预留的1MB   
    mov esi,edx

    xor eax,eax
    mov ax,bx
    mov ecx,0x10000      ;单位64KB
    mul ecx

    add esi,eax
    mov edx,esi
    jmp .mem_get_ok

    ;---------------- 方法3:使用中断0x15子功能0x88h获取内存信息 ----
.e801_failed_jmp_to_88:
    mov ah,0x88
    int 0x15
    jc .error_hlt
    and eax,0x0000ffff

    mov cx,0x400
    mul cx
    shl edx,16
    or edx,eax
    add edx,0x10000

.mem_get_ok:
    mov [total_mem_bytes],edx   

;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
; INT 0x10 功能号:0x13   使用中断打印字符串
;-------------------------------------------------------------------------------
;input:
;AH 子功能号=13H
;BH = 页码
;BL = 属性
;(DH,DL)=坐标(row,col)
;ES:BP=字符串地址
;AL=显示输出方式
;0 字符串中只含显示字符,显示属性在BL中,显示后光标位置不变
;1 字符串中只含显示字符,显示属性在BL中,显示后光标位置改变
;2 字符串中含显示字符和显示属性,显示后光标位置不变
;3 字符串中含显示字符和显示属性,显示后光标位置改变
;no return

    ;mov sp,LOADER_BASE_ADDR
    ;mov bp,loadermsg    ;es:bp 字符串地址
    ;mov cx,17           ;length of string
    ;mov ax,0x1301       ;指定功能号和属性
    ;mov bx,0x001f       ;指定页号(0x00)和颜色(0x1f)
    ;mov dx,0x1800
    ;int 0x10            ;中断

;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
; 准备进入保护模式
;-------------------------------------------------------------------------------
    ;step1 打开A20地址线
    ;step2 加载GDT
    ;step3  开启CR0的PE位


    ;step1
    in al,0x92
    or al,0000_0010b
    out 0x92,al


    ;step2
    lgdt [gdt_ptr]

    ;step3
    mov eax,cr0
    or eax,0x0000_0001
    mov cr0,eax

    jmp dword SELECTOR_CODE:p_mode_start    ;刷新流水线

;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
; 出错则挂起 
;-------------------------------------------------------------------------------
.error_hlt:
    hlt
;-------------------------------------------------------------------------------
    


;-------------------------------------------------------------------------------
; 以下已进入保护模式
;-------------------------------------------------------------------------------
[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax


    mov byte [gs:160],'p'   ; 屏幕上显示P,表示已经进入保护模式
    

    ;------- load kernel from hard disk -------
    mov eax,KERNEL_START_SECTOR
    mov ebx,KERNEL_BIN_BASE_ADDR
    mov ecx,200
    call rd_disk_m_32
    ;------------------------------------------


    ;------- 设置页表及开启分页 --------------
    call setup_page

    sgdt [gdt_ptr]  ;store gdt,将GDTR的内容放回到[gdt_ptr]


    mov ebx,[gdt_ptr + 2]                   ;获取GDT基址
    or dword [ebx + 0x18 + 4],0xc0000000    ;ebx+0x18为显存段描述符,
                                            ;ebx+0x18+4为显存段描述符的高32位,
                                            ;或上0xc0000000使其从内核空间的虚拟
                                            ;地址开始
                                            ;数据段和代码段不需要修改,因为它
                                            ;们在用户空间
                                            ;而显存段要访问内核空间
    
    add dword [gdt_ptr + 2],0xc0000000      ;GDT基址加上0xc0000000,成为内核空间
                                            ;的虚拟地址

    add esp,0xc0000000

    ;将页目录基址放到cr3
    mov eax,PAGE_DIR_TABLE_POS
    mov cr3,eax

    ;开启分页
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax
    ;分页功能已开启,以下访问的地址均为虚拟地址

    ;-----------------------------------------

    lgdt [gdt_ptr]  ;重新加载gdt的信息到GDTR

    mov byte [gs:160],'v'

    jmp SELECTOR_CODE:enter_kernel
enter_kernel:
    call kernel_init
    mov esp,0xc009f000
    mov byte [gs:160],'k'

    push .back
    jmp KERNEL_ENTRY_POINT
  .back:
    
    jmp $
;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
; 创建页目录及页表
;-------------------------------------------------------------------------------
setup_page:
    ;首先将页目录所占空间逐字节清零,总共4KB
    mov ecx,4096
    mov esi,0
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS+esi],0
    inc esi
    loop .clear_page_dir

    ;------------------------------------------------------------------------
    ;虚拟地址空间顶端1GB地址范围为0xc000_0000~0xffff_ffff),其底端4MB范围为
    ;0xc000_0000~0xc03f_ffff
    ;页目录表的物理地址范围为0x0010_0000~0x0010_0fff
    ;0号页表的物理地址为0x0010_1000~0x0010_1fff
    ;
    ;我们有4件事情要做
    ;1.分页后,0号页表所映射的4MB物理地址空间,包含了物理地址空间底端的1MB,这段
    ;  空间用于存放内核,不应该被其他数据破坏,因此我们需要为0号页表中的所有页
    ;  表项分配物理页,即底端的4MB,这样就不会被其他数据占用了,然后再将0号页表
    ;  的信息,填入0号页目录表项,以示其已经在内存中了
    ;
    ;2.为了方便通过虚拟地址访问内核空间,可以将虚拟地址空间顶端1GB中底端的
    ;  4MB即0xc000_0000~0xc03f_ffff映射到物理空间底端的4MB,,其中底端1MB为
    ;  0xc000_0000~0xc00f_ffff, 这样以来它们的低20位都是一一对应的,方便使用.
    ;  那么为了将这段空间映射到对应的物理地址空间,我们要将这段空间对应的
    ;  目录页表项映射到0号页表,这是因为0号页表映射的就是物理地址空间底端的
    ;  4MB,虚拟地址空间顶端1GB的4MB,对应的目录页号为其地址的高10位,
    ;  0xc000_0000~0xc03f_ffff的高10位为0x300,转换成10进制为768,也就是页目录
    ;  表的768号页目录表项,由于每一个页目录表大小为4bytes,那么0x300*4=0xc00
    ;  就得到了该页目录项相对于页目录表起始地址的偏移量,
    ;  0x0010_0000+0xc00=0x0010_0c00
    ;  就是该页目录表项的地址,我们将0号页表的信息填入目录页表768号.
    ;
    ;3.将目录项最后一项指向目录表本身
    ;
    ;4.从上面的分析可以看出,虚拟地址空间从0xc000_0000~0xffff_ffff的范围都属于
    ;  内核空间,为了让所有进程都能共享访问内核空间,我们需要实现将这段空间对应
    ;  的页目录表创建好,并写入每一项映射的页表信息,以便将来内核空间发生变化时,
    ;  都能实时访问到,而不需要再次同步页目录表
    ;------------------------------------------------------------------------

    ;开始创建页目录表
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x1000
    mov ebx,eax
    or eax, PG_US_U | PG_RW_W | PG_P

    mov [PAGE_DIR_TABLE_POS + 0x0],eax      ;1.将0号页表的信息,填入0号页目录表项
    mov [PAGE_DIR_TABLE_POS + 0xc00],eax    ;2.0号页表的信息填入768号目录页表项

    sub eax,0x1000
    mov [PAGE_DIR_TABLE_POS + 0xffc],eax    ;3.将目录项最后一项指向目录表本身


    ;1.为0号页表中的页表项分配内核空间底端1M物理页
    xor esi,esi
    mov edx,PG_US_U | PG_RW_W | PG_P        ;原先写的or 
    mov ecx,256                             ;1M/4K=256       
  .create_pte:
    mov [ebx+esi],edx
    add edx,0x1000                          ;一个物理页4KB
    add esi,4                               ;页表项一个4bytes
    loop .create_pte


    ;4.
    mov ebx,PAGE_DIR_TABLE_POS
    mov edx,ebx
    add edx,0x2000                          ;1号页表的偏移量
    or edx,PG_US_U | PG_RW_W | PG_P
    add ebx,0xc04                           ;769号页目录项的偏移量
    xor esi,esi
    mov ecx,254
  .create_kernel_pde:
    mov [ebx+esi],edx
    add edx,0x1000                          ;一个页表4KB
    add esi,4                               ;页目录表项一个4bytes
    loop .create_kernel_pde
    
    ret                                     ;返回
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; 内存逐字节拷贝mem_cpy(dst,src,size)
; 输入:栈中的3个参数,按从左到右的顺序push
; 输出:无
;-------------------------------------------------------------------------------
mem_cpy:
    cld                 ;clear direction flag
    push ebp
    mov ebp,esp
    push ecx            ;外层循环使用了ecx,push保护现场
    push esi            ;外层使用了esi 

    mov edi,[ebp + 8]
    mov esi,[ebp + 12]
    mov ecx,[ebp + 16]
    rep movsb

    pop esi
    pop ecx
    pop ebp
    ret
;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
; 将内核kernel.bin中的segment移动到指定的位置
;-------------------------------------------------------------------------------
kernel_init:
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    xor esi,esi

    mov ebx,KERNEL_BIN_BASE_ADDR
    mov esi,[ebx + 28]   ;get p_phoff; program header offset
    mov dx,[ebx + 42]   ;get e_phentsize: size of every item in program header table
    mov cx,[ebx + 44]   ;get e_phnum: program header num
  .move_each_segment:
    cmp byte [ebx + esi],PT_NULL
    je .PTNULL

    ;push arg1
    push dword [ebx + esi + 16]   ;push p_memsz: memory size

    ;push arg2
    mov eax,KERNEL_BIN_BASE_ADDR
    add eax,[ebx + esi + 4] ;add p_offset
    push eax                ;push KERNEL_BIN_BASE_ADDR + p_offset

    ;push arg3
    push dword [ebx + esi + 8]    ;push p_vaddr: virtual adrress
    call mem_cpy
    add esp,12              ;clean args
.PTNULL:
    add esi,edx             ;jump to next program header


    loop .move_each_segment
    ret

;-------------------------------------------------------------------------------



;-------------------------------------------------------------------------------
;function: read n sectors to somewhere you assign (32-bit version)
;input:
;   eax = LBA sector number
;   ebx  = the position that the data will be placed
;   ecx  = the numbers of sector that have not been read yet
;-------------------------------------------------------------------------------
rd_disk_m_32:
    mov esi,eax     ; push regs'
    mov di,cx
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
    mov [ebx],ax        ;与实模式下的区别是ebx替代了bx
    add ebx,2           ;因为寻址范围变大了,所以应该使用32位的寄存器
    loop .go_on_read

    ret     ; 调用返回
;-------------------------------------------------------------------------------
