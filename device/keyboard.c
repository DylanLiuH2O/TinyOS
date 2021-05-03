#include "device/keyboard.h"
#include "lib/kernel/print.h"
#include "lib/kernel/io.h"
#include "kernel/interrupt.h"
#include "kernel/global.h"

typedef uint8_t _BOOL;
#define _TRUE  1
#define _FALSE 0

#define KBD_BUF_PORT 0x60

/*control characters*/
#define esc       '\033'
#define backspace '\b'
#define tab       '\t'
#define enter     '\r'
#define delete    '\177'

#define char_invisible 0
#define ctrl_l_char    char_invisible
#define ctrl_r_char    char_invisible
#define shift_l_char   char_invisible
#define shift_r_char   char_invisible
#define alt_l_char     char_invisible
#define alt_r_char     char_invisible
#define caps_lock_char char_invisible

/*make and break of control characters*/
#define shift_l_make    0x2a
#define shift_r_make    0x36
#define alt_l_make      0x38
#define alt_r_make      0xe038
#define alt_r_break     0xe0b8
#define ctrl_l_make     0x1d
#define ctrl_r_make     0xe01d
#define ctrl_r_break    0xe09d
#define caps_lock_make  0x3a

static char keymap[][2] = {
    {0, 0},     //0x00
    {esc, esc}, //0x01
    {'1', '!'}, //0x02
    {'2', '@'}, //0x03
    {'3', '#'}, //0x04
    {'4', '$'}, //0x05
    {'5', '%'}, //0x06
    {'6', '^'}, //0x07
    {'7', '&'}, //0x08
    {'8', '*'}, //0x09
    {'9', '('}, //0x0a
    {'0', ')'}, //0x0b
    {'-', '_'}, //0x0c
    {'=', '+'}, //0x0d
    {backspace, backspace}, //0x0e
    {tab, tab}, //0x0f
    {'q', 'Q'}, //0x10
    {'w', 'W'}, //0x11
    {'e', 'E'}, //0x12
    {'r', 'R'}, //0x13
    {'t', 'T'}, //0x14
    {'y', 'Y'}, //0x15
    {'u', 'U'}, //0x16
    {'i', 'I'}, //0x17
    {'o', 'O'}, //0x18
    {'p', 'P'}, //0x19
    {'[', '{'}, //0x1a
    {']', '}'}, //0x1b
    {enter, enter}, //0x1c
    {ctrl_l_char, ctrl_l_char}, //0x1d
    {'a', 'A'}, //0x1e
    {'s', 'S'}, //0x1f
    {'d', 'D'}, //0x20
    {'f', 'F'}, //0x21
    {'g', 'G'}, //0x22
    {'h', 'H'}, //0x23
    {'j', 'J'}, //0x24
    {'k', 'K'}, //0x25
    {'l', 'L'}, //0x26
    {';', ':'}, //0x27
    {'\'', '"'},//0x28
    {'`', '~'}, //0x29
    {shift_l_char, shift_l_char}, //0x2a
    {'\\', '|'},//0x2b
    {'z', 'Z'}, //0x2c
    {'x', 'X'}, //0x2d
    {'c', 'C'}, //0x2e
    {'v', 'V'}, //0x2f
    {'b', 'B'}, //0x30
    {'n', 'N'}, //0x31
    {'m', 'M'}, //0x32
    {',', '<'}, //0x33
    {'.', '>'}, //0x34
    {'/', '?'}, //0x35
    {shift_r_char, shift_r_char},    //0x36
    {'*', '*'},                      //0x37
    {alt_l_char, alt_r_char},        //0x38
    {' ', ' '},                      //0x39
    {caps_lock_char, caps_lock_char} //0x3a
};

/*record status of buttons*/
static _BOOL ctrl_status;
static _BOOL shift_status;
static _BOOL alt_status;
static _BOOL caps_lock_status;
static _BOOL ext_scancode;      //is extend code

static void intr_keyboard_handler(void)
{
    // put_char('k');

    // uint8_t scancode = inb(KBD_BUF_PORT);
    // put_int(scancode);
    // put_char(' ');
    // inb(KBD_BUF_PORT);

    _BOOL last_ctrl       = ctrl_status;
    _BOOL last_shift      = shift_status;
    _BOOL last_caps_lock  = caps_lock_status;

    _BOOL is_break_code;
    uint16_t scancode = inb(KBD_BUF_PORT);

    if (scancode == 0xe0) {
        ext_scancode = _TRUE;
        return;
    }

    if (ext_scancode) {
        scancode |= 0xe000;
        ext_scancode = _FALSE;
    }

    is_break_code = (scancode & 0x80) != 0;

    //deal with the break code
    if (is_break_code == _TRUE) {
        uint16_t make_code = (scancode &= 0xff7f);

        if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
            ctrl_status = _FALSE;
        } else if (make_code == shift_l_make || make_code == shift_r_make) {
            shift_status = _FALSE;
        } else if (make_code == alt_l_make || make_code == alt_r_make) {
            alt_status = _FALSE;
        }

        return;

    //deal with the make code (alt_r_make, ctrl_r_make and code from 0x00 to 0x3b)
    //be careful, alt_l_make and ctrl_l_make are included in 0x01~0x3a
    } else if (scancode == alt_r_make || scancode == ctrl_r_make || 
                      (0x00 < scancode && scancode < 0x3b)) {
        _BOOL is_shift_down = _FALSE;

        //换挡后仍非字母的键
        if ((scancode <= 0x0d || scancode == 0x1a ||
             scancode == 0x1b || scancode == 0x27 ||
             scancode == 0x28 || scancode == 0x29 ||
             scancode == 0x2b || scancode == 0x33 ||
             scancode == 0x34 || scancode == 0x35) && 
             last_shift == _TRUE) {
            is_shift_down = _TRUE;

        //字母键及换挡不变的键
        } else if (last_shift == _TRUE  && last_caps_lock == _FALSE || 
                   last_shift == _FALSE && last_caps_lock == _TRUE) {
            is_shift_down = _TRUE;
        }

        uint8_t index = (scancode &= 0xff);
        char curr_char = keymap[index][is_shift_down];

        //打印非空字符
        if (curr_char > 0x0) {
            put_char(curr_char);

        //ctrl, shift, alt, caps_lock均被映射为空字符
        } else {
            if (scancode == ctrl_l_make || scancode == ctrl_r_make) {
                ctrl_status = _TRUE;
            } else if (scancode == shift_l_make || scancode == shift_r_make) {
                shift_status = _TRUE;
            } else if (scancode == alt_l_make || scancode == alt_r_make) {
                alt_status = _TRUE;
            } else if (scancode == caps_lock_make) {
                caps_lock_status = !caps_lock_status;
            }
        }

    } else {
        put_str("unknown key\n");
    }
}

void keyboard_init(void)
{
    put_str("[keyboard]: init start\n");
    register_handler(0x21, intr_keyboard_handler);
    put_str("[keyboard]: init done\n");
}
