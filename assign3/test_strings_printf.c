/* File: test_strings_printf.c
 * ---------------------------
 * ***** This file includes tests for strings.c in assignment 3; run make test *****
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

    // Test basic functionality
    memset(buf, '@', bufsize);
    for (int i = 0; i < bufsize; i++)
        assert(buf[i] == '@');

    // Test setting to 0 (common use case)
    memset(buf, 0, bufsize);
    for (int i = 0; i < bufsize; i++)
        assert(buf[i] == 0);

    // Test setting to -1 (all bits set)
    memset(buf, -1, bufsize);
    for (int i = 0; i < bufsize; i++)
        assert((unsigned char)buf[i] == 255);

    // Test partial buffer fill
    memset(buf, 'A', 10);
    for (int i = 0; i < 10; i++)
        assert(buf[i] == 'A');
    for (int i = 10; i < bufsize; i++)
        assert((unsigned char)buf[i] == 255);  // Ensure rest of buffer unchanged

    // Test single byte set
    memset(buf, 'B', 1);
    assert(buf[0] == 'B');
    assert(buf[1] == 'A');  // Ensure only first byte changed

    // Test empty set (should do nothing)
    memset(buf, 'C', 0);
    assert(buf[0] == 'B');  // Ensure buffer unchanged

    // Test with different data types
    int int_buf[5];
    memset(int_buf, 0, sizeof(int_buf));
    for (int i = 0; i < 5; i++)
        assert(int_buf[i] == 0);

    // Test setting to a multi-byte pattern (implementation-defined behavior)
    // This might not work as expected on all systems
    memset(buf, 0xAB, bufsize);
    for (int i = 0; i < bufsize; i++)
        assert((unsigned char)buf[i] == 0xAB);
}

static void test_strcmp(void) {
    assert(strcmp("apple", "apple") == 0);
    assert(strcmp("apple", "applesauce") < 0);
    assert(strcmp("pears", "apples") > 0);
    
    // New test cases
    assert(strcmp("", "") == 0);  // Empty strings
    assert(strcmp("a", "") > 0);  // Empty string comparison
    assert(strcmp("", "a") < 0);  // Empty string comparison
    assert(strcmp("abc", "abd") < 0);  // Differ at last character
    assert(strcmp("abc", "ab") > 0);  // One string is prefix of another
    assert(strcmp("abc", "abc\0def") == 0);  // Null terminator handling
    assert(strcmp("\0", "\0") == 0);  // Null terminators only
    assert(strcmp("ABC", "abc") != 0);  // Case sensitivity
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
    
    // New test cases
    memset(buf, '@', bufsize);
    buf[0] = '\0';
    
    assert(strlcat(buf, "", bufsize) == 0);  // Appending empty string
    assert(strlen(buf) == 0);
    
    assert(strlcat(buf, "Hello", 6) == 5);  // Exactly filling the buffer
    assert(strcmp(buf, "Hello") == 0);
    
    assert(strlcat(buf, " World", 8) == 11);  // Partial append due to size limit
    assert(strcmp(buf, "Hello W") == 0);
    
    char small_buf[5] = "";
    assert(strlcat(small_buf, "Toolong", sizeof(small_buf)) == 7);  // Truncation
    assert(strcmp(small_buf, "Tool") == 0);
    
    char full_buf[5] = "Full";
    assert(strlcat(full_buf, "Extra", sizeof(full_buf)) == 9);  // Appending to full buffer
    assert(strcmp(full_buf, "Full") == 0);
}

static void test_strtonum(void) {
    long val;
    const char *rest = NULL;

    val = strtonum("013", NULL);
    assert(val == 13);

    const char *input = "107rocks";
    val = strtonum(input, &rest);
    assert(val == 107);
    assert(rest == &input[3]);
    
    // Updated test cases for unsigned strtonum
    assert(strtonum("0", NULL) == 0);  // Zero
    assert(strtonum("42", NULL) == 42);  // Positive number
    assert(strtonum("4294967295", NULL) == 4294967295UL);  // Max 32-bit unsigned integer
    
    const char *overflow_input = "18446744073709551615"; // Max unsigned long (64-bit)
    rest = NULL;
    val = strtonum(overflow_input, &rest);
    assert(val == 18446744073709551615UL);
    assert(rest == overflow_input + 20);  // Stopped at the end
    
    const char *mixed_input = "42abc123";
    rest = NULL;
    val = strtonum(mixed_input, &rest);
    assert(val == 42);
    assert(rest == mixed_input + 2);
    
    const char *invalid_input = "notanumber";
    rest = NULL;
    val = strtonum(invalid_input, &rest);
    assert(val == 0);
    assert(rest == invalid_input);
    
    const char *space_input = "  123";
    rest = NULL;
    val = strtonum(space_input, &rest);
    assert(val == 123);
    assert(rest == space_input + 5);  // strtonum should skip whitespace
}

static void test_helpers(void) {
    char buf[64];  // Increased buffer size for larger numbers
    size_t bufsize = sizeof(buf);
    memset(buf, '@', bufsize);

    num_to_string(209, 10, buf);
    assert(strcmp(buf, "209") == 0);
    num_to_string(209, 16, buf);
    assert(strcmp(buf, "d1") == 0);

    assert(strcmp(decimal_string(-99), "-99") == 0);
    assert(strcmp(hex_string(0x107e), "107e") == 0);

    // Additional tests for num_to_string
    num_to_string(0, 10, buf);
    assert(strcmp(buf, "0") == 0);
    
    num_to_string(4294967295, 10, buf);  // Max 32-bit unsigned int
    assert(strcmp(buf, "4294967295") == 0);
    
    num_to_string(18446744073709551615UL, 10, buf);  // Max 64-bit unsigned long
    assert(strcmp(buf, "18446744073709551615") == 0);
    
    num_to_string(255, 16, buf);
    assert(strcmp(buf, "ff") == 0);
    
    num_to_string(0xdeadbeef, 16, buf);
    assert(strcmp(buf, "deadbeef") == 0);
    
    num_to_string(0, 2, buf);
    assert(strcmp(buf, "0") == 0);
    
    num_to_string(15, 2, buf);
    assert(strcmp(buf, "1111") == 0);
    
    num_to_string(42, 8, buf);
    assert(strcmp(buf, "52") == 0);
    
    num_to_string(1234567890, 36, buf);
    assert(strcmp(buf, "kf12oi") == 0);

    // Additional tests for decimal_string
    assert(strcmp(decimal_string(0), "0") == 0);
    assert(strcmp(decimal_string(2147483647), "2147483647") == 0);  // Max 32-bit signed int
    assert(strcmp(decimal_string(-2147483648), "-2147483648") == 0);  // Min 32-bit signed int

    // Additional tests for hex_string
    assert(strcmp(hex_string(0), "0") == 0);
    assert(strcmp(hex_string(255), "ff") == 0);
    assert(strcmp(hex_string(0xdeadbeef), "deadbeef") == 0);

    // Additional edge cases for num_to_string
    num_to_string(1, 10, buf);
    assert(strcmp(buf, "1") == 0);
    
    num_to_string(9223372036854775807UL, 10, buf);  // Max 63-bit unsigned long
    assert(strcmp(buf, "9223372036854775807") == 0);
    
    num_to_string(18446744073709551615UL, 16, buf);  // Max 64-bit unsigned long in hex
    assert(strcmp(buf, "ffffffffffffffff") == 0);
    
    num_to_string(18446744073709551615UL, 2, buf);  // Max 64-bit unsigned long in binary
    assert(strcmp(buf, "1111111111111111111111111111111111111111111111111111111111111111") == 0);
    
    num_to_string(18446744073709551615UL, 36, buf);  // Max 64-bit unsigned long in base 36
    assert(strcmp(buf, "3w5e11264sgsf") == 0);

    // Additional edge cases for decimal_string
    assert(strcmp(decimal_string(1), "1") == 0);
    assert(strcmp(decimal_string(-1), "-1") == 0);
    assert(strcmp(decimal_string(9223372036854775807L), "9223372036854775807") == 0);  // Max 63-bit signed long
    assert(strcmp(decimal_string(-9223372036854775807L - 1), "-9223372036854775808") == 0);  // Min 64-bit signed long

    // Additional edge cases for hex_string
    assert(strcmp(hex_string(1), "1") == 0);
    assert(strcmp(hex_string(15), "f") == 0);
    assert(strcmp(hex_string(16), "10") == 0);
    assert(strcmp(hex_string(9223372036854775807UL), "7fffffffffffffff") == 0);  // Max 63-bit unsigned long
    assert(strcmp(hex_string(18446744073709551615UL), "ffffffffffffffff") == 0);  // Max 64-bit unsigned long
}

static void test_snprintf(void) {
    char buf[100];
    size_t bufsize = sizeof(buf);
    int result;

    // Test basic string
    result = snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);
    assert(result == 13);

    // Test truncation
    char truncbuf[10];
    result = snprintf(truncbuf, sizeof(truncbuf), "Truncate: %s", "This is a long string");
    assert(strlen(truncbuf) == 9);  // 9 characters + null terminator
    assert(truncbuf[9] == '\0');    // Ensure null termination
    assert(result == 31);  // Total length if buffer was large enough

    // Test truncation with exact buffer size
    result = snprintf(buf, 10, "Truncate: %s", "This is a long string");
    assert(strcmp(buf, "Truncate:") == 0);
    assert(result == 31);  // Total length if buffer was large enough

    // Test %c
    result = snprintf(buf, bufsize, "Char: %c", 'A');
    assert(strcmp(buf, "Char: A") == 0);
    assert(result == 7);

    // Test %s
    result = snprintf(buf, bufsize, "String: %s", "test");
    assert(strcmp(buf, "String: test") == 0);
    assert(result == 12);

    // Test %d
    result = snprintf(buf, bufsize, "Integer: %d", 12345);
    assert(strcmp(buf, "Integer: 12345") == 0);
    assert(result == 14);

    // Test %ld
    result = snprintf(buf, bufsize, "Long: %ld", 1234567890L);
    assert(strcmp(buf, "Long: 1234567890") == 0);
    assert(result == 16);

    // Test %x
    result = snprintf(buf, bufsize, "Hex: %x", 0xABCD);
    assert(strcmp(buf, "Hex: abcd") == 0);
    assert(result == 9);

    // Test %lx
    result = snprintf(buf, bufsize, "Long Hex: %lx", 0x123456789ABCDEFL);
    assert(strcmp(buf, "Long Hex: 123456789abcdef") == 0);
    assert(result == 25);

    // Test %p
    void *ptr = (void *)0x1234;
    result = snprintf(buf, bufsize, "Pointer: %p", ptr);
    assert(strcmp(buf, "Pointer: 0x00001234") == 0);
    assert(result == 19);

    // Test %%
    result = snprintf(buf, bufsize, "Percent: %%");
    assert(strcmp(buf, "Percent: %") == 0);
    assert(result == 10);

    // Test basic field width
    result = snprintf(buf, bufsize, "%5d", 42);
    assert(strcmp(buf, "   42") == 0);
    assert(result == 5);

    // Test field width
    result = snprintf(buf, bufsize, "Width: %5d", 42);
    assert(strcmp(buf, "Width:    42") == 0);
    assert(result == 12);

    // Test zero padding
    result = snprintf(buf, bufsize, "Zero-pad: %05d", 42);
    assert(strcmp(buf, "Zero-pad: 00042") == 0);
    assert(result == 15);

    // Test field width with spaces
    result = snprintf(buf, bufsize, "Spaced: %5d", 123);
    assert(strcmp(buf, "Spaced:   123") == 0);
    assert(result == 13);

    // Test field width with larger number
    result = snprintf(buf, bufsize, "Large: %10d", 12345);
    assert(strcmp(buf, "Large:      12345") == 0);
    assert(result == 17);

    // Test zero padding with larger number
    result = snprintf(buf, bufsize, "Large Zero: %010d", 12345);
    assert(strcmp(buf, "Large Zero: 0000012345") == 0);
    assert(result == 22);

    // Test field width with string
    result = snprintf(buf, bufsize, "String: %10s", "test");
    assert(strcmp(buf, "String:       test") == 0);
    assert(result == 18);

    // Test field width with character
    result = snprintf(buf, bufsize, "Char: %5c", 'A');
    assert(strcmp(buf, "Char:     A") == 0);
    assert(result == 11);

    // Test field width with hexadecimal
    result = snprintf(buf, bufsize, "Hex: %8x", 0xABC);
    assert(strcmp(buf, "Hex:      abc") == 0);
    assert(result == 13);

    // Test zero padding with hexadecimal
    result = snprintf(buf, bufsize, "Hex Zero: %08x", 0xABC);
    assert(strcmp(buf, "Hex Zero: 00000abc") == 0);
    assert(result == 18);

    // Test pointer formatting
    result = snprintf(buf, bufsize, "Ptr: %p", ptr);
    assert(strcmp(buf, "Ptr: 0x00001234") == 0);
    assert(result == 15);

    // Test multiple format specifiers
    result = snprintf(buf, bufsize, "Multi: %c %s %d %x", 'A', "test", 42, 0xFF);
    assert(strcmp(buf, "Multi: A test 42 ff") == 0);
    assert(result == 19);

    // Test buffer size of 0
    result = snprintf(NULL, 0, "Test %d", 123);
    assert(result == 8);

    // Test very large number
    result = snprintf(buf, bufsize, "Large: %ld", 9223372036854775807L);
    assert(strcmp(buf, "Large: 9223372036854775807") == 0);
    assert(result == 26);

    // Test negative numbers
    result = snprintf(buf, bufsize, "Negative: %d", -12345);
    assert(strcmp(buf, "Negative: -12345") == 0);
    assert(result == 16);

    // Test multiple pointers in one format string
    void *ptr1 = (void *)0x1000;
    void *ptr2 = (void *)0x2000;
    result = snprintf(buf, bufsize, "Pointers: %p and %p", ptr1, ptr2);
    assert(strcmp(buf, "Pointers: 0x00001000 and 0x00002000") == 0);
    assert(result == 35);

    // Test pointer with small buffer size (truncation test)
    void *small_buf_ptr = (void *)0x3000;
    char small_buf[15];
    result = snprintf(small_buf, sizeof(small_buf), "Ptr: %p", small_buf_ptr);
    assert(strcmp(small_buf, "Ptr: 0x0000300") == 0);
    assert(result == 15);

    uart_putstring("All snprintf tests passed!\n");
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

static void test_snprintf_multi_args(void) {
    char buf[200];
    size_t bufsize = sizeof(buf);
    int result;

    // Test 1: Multiple types
    result = snprintf(buf, bufsize, "%d %c %s %x %p", 42, 'A', "hello", 0xDEAD, (void*)0x1234);
    assert(strcmp(buf, "42 A hello dead 0x00001234") == 0);
    assert(result == 26);

    // Test 2: Field widths
    result = snprintf(buf, bufsize, "%5d %10s %5c", 123, "test", 'X');
    assert(strcmp(buf, "  123       test     X") == 0);
    assert(result == 22);

    // Test 3: Zero padding
    result = snprintf(buf, bufsize, "%05d %05x", 42, 0xAB);
    assert(strcmp(buf, "00042 000ab") == 0);
    assert(result == 11);

    // Test 4: Long and unsigned long
    result = snprintf(buf, bufsize, "%ld %lx", 1234567890L, 0xABCDEF01UL);
    assert(strcmp(buf, "1234567890 abcdef01") == 0);
    assert(result == 19);

    // Test 5: Multiple pointers
    void *ptr1 = (void*)0x1000, *ptr2 = (void*)0x2000;
    result = snprintf(buf, bufsize, "%p %p", ptr1, ptr2);
    assert(strcmp(buf, "0x00001000 0x00002000") == 0);
    assert(result == 21);

    // Test 6: Mixing strings and numbers
    result = snprintf(buf, bufsize, "%s: %d, %s: %x", "Int", -123, "Hex", 456);
    assert(strcmp(buf, "Int: -123, Hex: 1c8") == 0);
    assert(result == 19);

    // Test 7: Character arrays
    result = snprintf(buf, bufsize, "%c%c%c%c", 'T', 'E', 'S', 'T');
    assert(strcmp(buf, "TEST") == 0);
    assert(result == 4);

    // Test 8: Repeated format specifiers
    result = snprintf(buf, bufsize, "%d %d %d %d %d", 1, 2, 3, 4, 5);
    assert(strcmp(buf, "1 2 3 4 5") == 0);
    assert(result == 9);

    // Test 9: Mixed case format specifiers
    result = snprintf(buf, bufsize, "%d %ld %x %lx", 10, 20L, 30U, 40UL);
    assert(strcmp(buf, "10 20 1e 28") == 0);
    assert(result == 11);

    // Test 10: Truncation
    result = snprintf(buf, 10, "This is a long string that will be truncated");
    assert(strcmp(buf, "This is a") == 0);
    assert(result == 44);

    // Test 11: No arguments
    result = snprintf(buf, bufsize, "No arguments here!");
    assert(strcmp(buf, "No arguments here!") == 0);
    assert(result == 18);

    // Test 12: Escape sequences
    result = snprintf(buf, bufsize, "Newline\nTab\tBackslash\\");
    assert(strcmp(buf, "Newline\nTab\tBackslash\\") == 0);
    assert(result == 22);

    // Test 13: Combination of various specifiers
    result = snprintf(buf, bufsize, "%s %d %x %c %ld %lx %p %%", 
                      "Combined", 42, 0xFF, 'Z', 12345L, 0xABCDEFL, (void*)0x1234);
    assert(strcmp(buf, "Combined 42 ff Z 12345 abcdef 0x00001234 %") == 0);
    assert(result == 42);

    uart_putstring("All multi-argument snprintf tests passed!\n");
}

void test_printf(void) {
    // Test basic string
    printf("Hello, world!\n");

    // Test integer formatting
    printf("Decimal: %d, Hexadecimal: %x\n", 42, 42);

    // Test character
    printf("Character: %c\n", 'A');

    // Test string
    printf("String: %s\n", "test string");

    // Test padding
    printf("Padded integer: %05d\n", 123);

    // Test long integer
    printf("Long decimal: %ld, Long hex: %lx\n", 1234567890L, 0xABCDEF01L);

    // Test pointer
    int x = 10;
    printf("Pointer: %p\n", (void*)&x);

    // Test multiple arguments
    printf("Multiple args: %d %s %c %x\n", 10, "hello", 'Z', 255);

    // Test edge cases
    printf("Empty string: %s\n", "");
    printf("Zero: %d\n", 0);
    printf("Negative: %d\n", -42);
    printf("Percent sign: %%\n");

    // Test buffer overflow (assuming MAX_OUTPUT_LEN is 1024)
    char long_string[2000];
    for (int i = 0; i < 1999; i++) long_string[i] = 'A';
    long_string[1999] = '\0';
    printf("Long string: %s\n", long_string);
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
    test_snprintf_multi_args();
    test_printf();
    // test_disassemble(); // uncomment if you implement extension

    uart_putstring("Successfully finished executing main() in test_strings_printf.c\n");
}
