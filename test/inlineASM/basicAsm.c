#include <stdio.h>
char* str = "Hello World\n";
int count = 0;
int main() {
    __asm__ ("pusha;\
         movl $4,%eax;\
         movl $1,%ebx;\
         movl str,%ecx;\
         movl $12,%edx;\
         int $0x80;\
         mov %eax,count;\
         popa");
    printf("count=%d\n", count);
    return 0;
}
