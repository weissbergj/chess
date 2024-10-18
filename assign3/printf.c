/* File: printf.c
 * --------------
 * ***** This is the printf function in C *****
 */
#include "printf.h"
#include <stdarg.h>
#include <stdint.h>
#include "strings.h"

/* Prototypes for internal helpers.
 * Typically we would qualify these functions as static (private to module)
 * but in order to call them from the test program, must declare externally
 */
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);

// max number of digits in long + space for negative sign and null-terminator
#define MAX_DIGITS 25


/* Convenience functions `hex_string` and `decimal_string` are provided
 * to you.  You should use the functions as-is, do not change the code!
 *
 * A key implementation detail to note is these functions declare
 * a buffer to hold the output string and return the address of buffer
 * to the caller. If that buffer memory were located on stack, it would be
 * incorrect to use pointer after function exit because local variables
 * are deallocated. To ensure the buffer memory is accessible after
 * the function exists, the declaration is qualified `static`. Memory
 * for a static variable is not stored on stack, but instead in the global data
 * section, which exists outside of any function call. Additionally static
 * makes it so there is a single copy of the variable, which is shared by all
 * calls to the function. Each time you call the function, it overwrites/reuses
 * the same variable/memory.
 *
 * Adding static qualifier to a variable declared inside a function is a
 * highly atypical practice and appropriate only in very specific situations.
 * You will likely never need to do this yourself.
 * Come talk to us if you want to know more!
 */

const char *hex_string(unsigned long val) {
    // static buffer to allow use after function returns (see note above)
    static char buf[MAX_DIGITS];
    num_to_string(val, 16, buf); // num_to_string does the hard work
    return buf;
}

const char *decimal_string(long val) {
    // static buffer to allow use after function returns (see note above)
    static char buf[MAX_DIGITS];
    if (val < 0) {
        buf[0] = '-';   // add negative sign in front first
        num_to_string(-val, 10, buf + 1); // pass positive val as arg, start writing at buf + 1
    } else {
        num_to_string(val, 10, buf);
    }
    return buf;
}

void num_to_string(unsigned long num, int base, char *outstr) {
    int digits_in_num = 0;
    unsigned long temp = num;

    do {
        temp /= base;
        digits_in_num++;
    } while (temp > 0);     // makes sure the 0-case has one digit_in_num

    for (int i = digits_in_num - 1; i >= 0; i--) {
        int modulo = num % base;
        outstr[i] = (char)(modulo <=9 ? modulo + '0' : modulo + 'a' - 10); //if 0-9 add ASCII 0; else add 'a' - 10
        num /= base;
    }
    outstr[digits_in_num] = '\0';
}

int parser(const char **str) {
    int result = 0;
    while (**str >= '0' && **str <= '9') {
        result = result * 10 + (**str - '0');
        (*str)++;
    }
    return result;
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {
    size_t i = 0, j = 0;
    int l = 0; //these are the characters we would have written had it not stopped

    void add_char(char c) {
        if (j < bufsize - 1) {
            buf[j++] = c;
        }
        l++;
    }

    void add_padding(int count, char pad_char) {
        for (int k = 0; k < count; k++) {
            add_char(pad_char);
        }
    }

    while (format[i] != '\0') {
        if (format[i] == '%' && format[i+1] != '\0') {
            i++;
            int field_width = 0;
            int zero_pad = 0;
            
            if (format[i] == '0') {
                zero_pad = 1;
                i++;
            }
            if (format[i] >= '1' && format[i] <= '9') {
                const char *width_start = &format[i];
                field_width = parser(&width_start);
                i += width_start - &format[i];
            }

            switch (format[i]) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    int padding = (field_width > 1) ? field_width - 1 : 0;
                    add_padding(padding, zero_pad ? '0' : ' ');
                    add_char(c);
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, char*);
                    int str_len = strlen(str);
                    int padding = (field_width > str_len) ? field_width - str_len : 0;
                    add_padding(padding, zero_pad ? '0' : ' ');
                    while (*str) {
                        add_char(*str++);
                    }
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    const char *str = decimal_string(d);
                    int str_len = strlen(str);
                    int padding = (field_width > str_len) ? field_width - str_len : 0;
                    add_padding(padding, zero_pad ? '0' : ' ');
                    while (*str) {
                        add_char(*str++);
                    }
                    break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    const char *str = hex_string(x);
                    int str_len = strlen(str);
                    int padding = (field_width > str_len) ? field_width - str_len : 0;
                    add_padding(padding, zero_pad ? '0' : ' ');
                    while (*str) {
                        add_char(*str++);
                    }
                    break;
                }
                case 'l': {
                    i++;
                    if (format[i] == 'd') {
                        long ld = va_arg(args, long);
                        const char *str = decimal_string(ld);
                        int str_len = strlen(str);
                        int padding = (field_width > str_len) ? field_width - str_len : 0;
                        add_padding(padding, zero_pad ? '0' : ' ');
                        while (*str) {
                            add_char(*str++);
                        }
                    } else if (format[i] == 'x') {
                        unsigned long lx = va_arg(args, unsigned long);
                        const char *str = hex_string(lx);
                        int str_len = strlen(str);
                        int padding = (field_width > str_len) ? field_width - str_len : 0;
                        add_padding(padding, zero_pad ? '0' : ' ');
                        while (*str) {
                            add_char(*str++);
                        }
                    }
                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    unsigned long addr = (unsigned long)ptr;
                    
                    // ad 0x prefix
                    add_char('0');
                    add_char('x');
                    
                    // Output  address (8 hex , zero-padded)
                    for (int k = 7; k >= 0; k--) {
                        int addtl = (addr >> (k * 4)) & 0xF;
                        char hex_digit = (addtl < 10) ? '0' + addtl : 'a' + (addtl - 10);
                        add_char(hex_digit);
                    }
                    break;
                }
                case '%': {
                    add_char('%');
                    break;
                }
            }
        } else {
            add_char(format[i]);
        }
        i++;
    }

    if (bufsize > 0) {
        buf[j < bufsize - 1 ? j : bufsize - 1] = '\0';
    }
    return l;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int print = vsnprintf(buf, bufsize, format, args);
    va_end(args);
    return print;
}

// ok to assume printf output is never longer that MAX_OUTPUT_LEN
#define MAX_OUTPUT_LEN 1024

int printf(const char *format, ...) {
    char buf[MAX_OUTPUT_LEN];
    va_list args;
    va_start(args, format);
    int num_characters_written = vsnprintf(buf, MAX_OUTPUT_LEN, format, args);
    va_end(args);

    
    // uart_putstring(buf);
    // or do we do a loop through i < MAX_OUTPUT_LEN and do uart_putchar buf[i]

    return num_characters_written;
}


/* From here to end of file is some sample code and suggested approach
 * for those of you doing the disassemble extension. Otherwise, ignore!
 *
 * The struct insn bitfield is declared using exact same layout as bits are organized in
 * the encoded instruction. Accessing struct.field will extract just the bits
 * apportioned to that field. If you look at the assembly the compiler generates
 * to access a bitfield, you will see it simply masks/shifts for you. Neat!
 */
/*
static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                    "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };

struct insn  {
    uint32_t opcode: 7;
    uint32_t reg_d:  5;
    uint32_t funct3: 3;
    uint32_t reg_s1: 5;
    uint32_t reg_s2: 5;
    uint32_t funct7: 7;
};

void sample_use(unsigned int *addr) {
    struct insn in = *(struct insn *)addr;
    printf("opcode is 0x%x, reg_dst is %s\n", in.opcode, reg_names[in.reg_d]);
}
*/








