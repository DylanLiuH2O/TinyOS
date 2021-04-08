[bits 32]
section .text
global switch_to
switch_to:
    ;进入函数时,栈从顶往下的数据分别是:返回地址,curr_tread和next_thread
    push esi
    push edi
    push ebx
    push ebp

    mov eax,[esp+20]    ;获取执行函数前压入的参数curr_thread
    mov [eax],esp       ;将curr_thread->self_kstack置为esp


    mov eax,[esp+24]    ;获取next_thread
    mov esp,[eax]       ;将next_thread->self_kstack送入esp,切换栈环境

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret



