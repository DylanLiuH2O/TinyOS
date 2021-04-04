#include "print.h"
#include "init.h"
#include "debug.h"
#include "stdint.h"
#include "string.h"
#define DEBUG
int main(void) {
    put_str("I am kernel\n");
    init_all();
    //asm volatile("sti");    open interrupt
    //ASSERT(1 == 2);
    char a[10] = "abcdefg";
    char* b = "abcdefg";
    char* c = "abcdefi";
    char* d = "abcde";
    char* e = "abcdefh";
    char* f = "abcdeff";
    char* g = "xxxxxxxxx";
    char h[20] = "gggg";

    put_str("test memset:\n");
    memset(a, 'a', 7);
    put_str("  expect: aaaaaaa\n");
    put_str("  actual: ");
    put_str(a);
    put_char('\n');

    put_str("test memcpy:\n");
    memcpy(a, g, 10);
    put_str("  expect: xxxxxxxxx\n");
    put_str("  actual: ");
    put_str(a);
    put_char('\n');

    put_str("test memcmp:\n");
    uint32_t res = memcmp("abcd", "abce", 4);
    put_str("  expect: FFFFFFFF\n");
    put_str("  actual: ");
    put_int(res);
    put_char('\n');

    put_str("test strcpy:\n");
    strcpy(b, e);
    put_str("  expect: abcdefh\n");
    put_str("  actual: ");
    put_str(b);
    put_char('\n');

    put_str("test strlen:\n");
    uint32_t len = strlen(c);
    put_str("  expect: 7\n");
    put_str("  actual: ");
    put_int(len);
    put_char('\n');

    put_str("test strcmp:\n");
    res = strcmp("abcd", "abce");
    put_str("  expect: FFFFFFFF\n");
    put_str("  actual: ");
    put_int(res);
    put_char('\n');
    /*
    put_str("test strchr\n");
    
    put_str("  expect: aaaaaaa\n");
    put_str("  actual: ");
    put_str(a);
    put_char('\n');

    put_str("test strchr_r\n");
    put_str("  expect: aaaaaaa\n");
    put_str("  actual: ");
    put_str(a);
    put_char('\n');
    */

    put_str("test strcat\n");
    strcat(h, c);
    put_str("  expect: ggggabcdefi\n");
    put_str("  actual: ");
    put_str(h);
    put_char('\n');

    put_str("test str_count\n");
    res = str_count("abcdefgg", 'g');
    put_str("  expect: 2\n");
    put_str("  actual: ");
    put_int(res);
    put_char('\n');



    while(1);
}
