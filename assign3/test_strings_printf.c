/* File: test_strings_printf.c
 * ---------------------------
 * ***** TODO: add your file header comment here *****
 */
#include "assert.h"
#include "printf.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

// Prototypes copied from printf.c to allow unit testing of helper functions
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);


static void test_memset(void) {
    char buf[25];
    size_t bufsize = sizeof(buf);

    memset(buf, '@', bufsize); // fill buffer with repeating value
    for (int i = 0; i < bufsize; i++)
        assert(buf[i] == '@'); // confirm value
}

static void test_strcmp(void) {
    assert(strcmp("apple", "apple") == 0);
    assert(strcmp("apple", "applesauce") < 0);
    assert(strcmp("pears", "apples") > 0);
}

static void test_strlcat(void) {
    char buf[20];
    size_t bufsize = sizeof(buf);
    memset(buf, '@', bufsize); // init array contents with fixed repeat value

    buf[0] = '\0'; // null at first index makes empty string
    assert(strlen(buf) == 0);
    strlcat(buf, "CS", bufsize); // append CS
    assert(strlen(buf) == 2);
    assert(strcmp(buf, "CS") == 0);
    strlcat(buf, "107e", bufsize); // append 107e
    assert(strlen(buf) == 6);
    assert(strcmp(buf, "CS107e") == 0);
}

static void test_strtonum(void) {
    long val = strtonum("013", NULL);
    assert(val == 13);

    const char *input = "107rocks";
    const char *rest = NULL;

    val = strtonum(input, &rest);
    assert(val == 107);
    // rest was modified to point to first non-digit character
    assert(rest == &input[3]);
}

static void test_helpers(void) {
    char buf[32];
    size_t bufsize = sizeof(buf);
    memset(buf, '@', bufsize); // init array contents with fixed repeat value

    num_to_string(209, 10, buf);
    assert(strcmp(buf, "209") == 0);
    num_to_string(209, 16, buf);
    assert(strcmp(buf, "d1") == 0);

    assert(strcmp(decimal_string(-99), "-99") == 0);
    assert(strcmp(hex_string(0x107e), "107e") == 0);
}

static void test_snprintf(void) {
    char buf[100];
    size_t bufsize = sizeof(buf);
    memset(buf, '@', bufsize); // init array contents with fixed repeat value

    // Try no formatting codes
    snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);

    // String
    snprintf(buf, bufsize, "%s", "binky");
    assert(strcmp(buf, "binky") == 0);

    // Test return value
    assert(snprintf(buf, bufsize, "winky") == 5);
    assert(snprintf(buf, 2, "winky") == 5);

    // From here it is up to you...!
}


// This function just here as code to disassemble for extension
int sum(int n) {
    int result = 6;
    for (int i = 0; i < n; i++) {
        result += i * 3;
    }
    return result + 729;
}

void test_disassemble(void) {
    const unsigned int add =  0x00f706b3;
    const unsigned int xori = 0x0015c593;
    const unsigned int bne =  0xfe061ce3;
    const unsigned int sd =   0x02113423;

    // formatting code %pI accesses the disassemble extension.
    // If extension not implemented, regular version of printf
    // will simply output pointer address followed by I
    // e.g.  "... disassembles to 0x07ffffd4I"
    printf("Encoded instruction %08x disassembles to %pI\n", add, &add);
    printf("Encoded instruction %08x disassembles to %pI\n", xori, &xori);
    printf("Encoded instruction %08x disassembles to %pI\n", bne, &bne);
    printf("Encoded instruction %08x disassembles to %pI\n", sd, &sd);

    unsigned int *fn = (unsigned int *)sum; // disassemble instructions from sum function
    for (int i = 0; i < 10; i++) {
        printf("%p:  %08x  %pI\n", &fn[i], fn[i], &fn[i]);
    }
}

void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_strings_printf.c\n");

    test_memset();
    test_strcmp();
    test_strlcat();
    test_strtonum();
    test_helpers();
    test_snprintf();
    // test_disassemble(); // uncomment if you implement extension

    // TODO: Add more and better tests!

    uart_putstring("Successfully finished executing main() in test_strings_printf.c\n");
}
