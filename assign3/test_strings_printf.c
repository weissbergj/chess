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
    // char long_string[2000];
    // for (int i = 0; i < 1999; i++) long_string[i] = 'A';
    // long_string[1999] = '\0';
    // printf("Long string: %s\n", long_string);
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

    printf("Encoded instruction %08x disassembles to %pI\n", add, &add);
    printf("Encoded instruction %08x disassembles to %pI\n", xori, &xori);
    printf("Encoded instruction %08x disassembles to %pI\n", bne, &bne);
    printf("Encoded instruction %08x disassembles to %pI\n", sd, &sd);

    unsigned int *fn = (unsigned int *)sum; // disassemble instructions from sum function
    for (int i = 0; i < 10; i++) {
        printf("%p:  %08x  %pI\n", &fn[i], fn[i], &fn[i]);
    }
}

void test_disassemble2(void) {
    char buf[100];
    size_t bufsize = sizeof(buf);

    struct test_case {
        unsigned int instruction;
        const char *expected;
    };

    struct test_case tests[] = {
        {0x00f706b3, "ADD a3, a4, a5"},
        {0x40f706b3, "SUB a3, a4, a5"},
        {0x00f71633, "SLL a2, a4, a5"},
        {0x00f726b3, "SLT a3, a4, a5"},
        {0x00f736b3, "SLTU a3, a4, a5"},
        {0x00f746b3, "XOR a3, a4, a5"},
        {0x00f756b3, "SRL a3, a4, a5"},
        {0x40f756b3, "SRA a3, a4, a5"},
        {0x00f766b3, "OR a3, a4, a5"},
        {0x00f776b3, "AND a3, a4, a5"},
        {0x00f68693, "ADDI a3, a3, 15"},
        {0x00f6a693, "SLTI a3, a3, 15"},
        {0x00f6b693, "SLTIU a3, a3, 15"},
        {0x00f6c693, "XORI a3, a3, 15"},
        {0x00f6e693, "ORI a3, a3, 15"},
        {0x00f6f693, "ANDI a3, a3, 15"},
        {0x00f69693, "SLLI a3, a3, 15"},
        {0x00f6d693, "SRLI a3, a3, 15"},
        {0x40f6d693, "SRAI a3, a3, 15"},
        {0x00068683, "LB a3, 0(a3)"},
        {0x00069683, "LH a3, 0(a3)"},
        {0x0006a683, "LW a3, 0(a3)"},
        {0x0006c683, "LBU a3, 0(a3)"},
        {0x0006d683, "LHU a3, 0(a3)"},
        {0x00f68023, "SB a5, 0(a3)"},
        {0x00f69023, "SH a5, 0(a3)"},
        {0x00f6a023, "SW a5, 0(a3)"},
        {0x00f68063, "BEQ a3, a5, 0"},
        {0x00f69063, "BNE a3, a5, 0"},
        {0x00f6c063, "BLT a3, a5, 0"},
        {0x00f6d063, "BGE a3, a5, 0"},
        {0x00f6e063, "BLTU a3, a5, 0"},
        {0x00f6f063, "BGEU a3, a5, 0"},
        {0x000000ef, "JAL ra, 0"},
        {0x000680e7, "JALR ra, 0(a3)"},
        {0x000006b7, "LUI a3, 0"},
        {0x00000697, "AUIPC a3, 0"},
        {0x00000073, "ECALL"},
        {0x00100073, "EBREAK"},
        {0x0000000f, "FENCE"},
        {0x0000100f, "FENCE.I"},
        // RV32M instructions
        {0x02f70733, "MUL a4, a4, a5"},
        {0x02f71733, "MULH a4, a4, a5"},
        {0x02f72733, "MULHSU a4, a4, a5"},
        {0x02f73733, "MULHU a4, a4, a5"},
        {0x02f74733, "DIV a4, a4, a5"},
        {0x02f75733, "DIVU a4, a4, a5"},
        {0x02f76733, "REM a4, a4, a5"},
        {0x02f77733, "REMU a4, a4, a5"},

        // RV64M instructions
        {0x02f7073b, "MULW a4, a4, a5"},
        {0x02f7473b, "DIVW a4, a4, a5"},
        {0x02f7573b, "DIVUW a4, a4, a5"},
        {0x02f7673b, "REMW a4, a4, a5"},
        {0x02f7773b, "REMUW a4, a4, a5"},

        // Zicsr instructions
        {0x34102573, "CSRRS a0, mepc"}, 
        {0x34151073, "CSRRW mepc, a0"}, //fix values for mepc instead of 0x341 same with mcause, mtval
        {0x34252073, "CSRRS mepc, a0"},
        {0x34353073, "CSRRC mepc, a0"},
        {0x34145073, "CSRRWI mepc, 8"}, 
        {0x34246073, "CSRRSI mepc, 8"},
        {0x34347073, "CSRRCI mepc, 8"}, 

        // RV32A instructions
        {0x100522af, "LR.W t0, (a0)"},
        {0x18c5232f, "SC.W t1, a2, (a0)"},
        {0x08c5252f, "AMOSWAP.W a0, a2, (a0)"},
        {0x00c5202f, "AMOADD.W zero, a2, (a0)"},
        {0x20c5222f, "AMOXOR.W tp, a2, (a0)"},
        {0x60c5242f, "AMOAND.W s0, a2, (a0)"},
        {0x40c5262f, "AMOOR.W a2, a2, (a0)"},
        {0x80c5282f, "AMOMIN.W a6, a2, (a0)"},
        {0xa0c52a2f, "AMOMAX.W s4, a2, (a0)"},
        {0xc0c52c2f, "AMOMINU.W t8, a2, (a0)"},
        {0xe0c52e2f, "AMOMAXU.W t3, a2, (a0)"},

        // RV64A instructions
        {0x100532af, "LR.D t0, (a0)"},
        {0x18c5332f, "SC.D t1, a2, (a0)"},
        {0x08c5352f, "AMOSWAP.D a0, a2, (a0)"},
        {0x00c5302f, "AMOADD.D zero, a2, (a0)"},
        {0x20c5322f, "AMOXOR.D tp, a2, (a0)"},
        {0x60c5342f, "AMOAND.D s0, a2, (a0)"},
        {0x40c5362f, "AMOOR.D a2, a2, (a0)"},
        {0x80c5382f, "AMOMIN.D a6, a2, (a0)"},  
        {0xa0c53a2f, "AMOMAX.D s4, a2, (a0)"}, 
        {0xc0c53c2f, "AMOMINU.D t8, a2, (a0)"}, 
        {0xe0c53e2f, "AMOMAXU.D t4, a2, (a0)"}, 

        // RV32F instructions
        {0x00002507, "FLW a0, 0(zero)"}, //NOTE TEHSE HAVE ALL BEEN MODIFIED FOR f0/f1!!!
        {0x00a52627, "FSW a2, 12(a0)"},
        {0x00a5f553, "FADD.S a0, a1, a0, rne"}, 
        {0x08a5f553, "FSUB.S a0, a1, a0, rne"},
        {0x10a5f553, "FMUL.S a0, a1, a0, rne"},
        {0x18a5f553, "FDIV.S a0, a1, a0, rne"},
        {0x58057553, "FSQRT.S a0, a0, rne"},
        {0x20a58553, "FSGNJ.S a0, a1, a0"},
        {0x20a59553, "FSGNJN.S a0, a1, a0"},
        {0x20a5a553, "FSGNJX.S a0, a1, a0"},
        {0x28a58553, "FMIN.S a0, a1, a0"},
        {0x28a59553, "FMAX.S a0, a1, a0"},
        {0xa0a58553, "FLE.S a0, a1, a0"},
        {0xa0a59553, "FLT.S a0, a1, a0"},
        {0xa0a5a553, "FEQ.S a0, a1, a0"},
        {0xc0057553, "FCVT.W.S a0, a0, rne"},
        {0xc0157553, "FCVT.WU.S a0, a0, rne"},
        {0xe0050553, "FMV.X.W a0, a0"},
        {0xe0051553, "FCLASS.S a0, a0"},
        {0xd0057553, "FCVT.S.W a0, a0, rne"},
        {0xd0157553, "FCVT.S.WU a0, a0, rne"},
        {0xf0050553, "FMV.W.X a0, a0"},
        {0x00a5f543, "FMADD.S a0, a1, a0, a1, rne"},
        {0x00a5f547, "FMSUB.S a0, a1, a0, a1, rne"},
        {0x00a5f54b, "FNMSUB.S a0, a1, a0, a1, rne"},
        {0x00a5f54f, "FNMADD.S a0, a1, a0, a1, rne"},

        // RV64F instructions (additional to RV32F)
        {0xc0257553, "FCVT.L.S a0, a0, rne"},
        {0xc0357553, "FCVT.LU.S a0, a0, rne"},
        {0xd0257553, "FCVT.S.L a0, a0, rne"},
        {0xd0357553, "FCVT.S.LU a0, a0, rne"},

        // RV32D instructions
        {0x00003507, "FLD fa0, 0(zero)"},
        {0x00a53027, "FSD fa0, 0(a0)"},
        {0x02a5f543, "FMADD.D fa0, fa1, fa0, fa1, rne"}, //this might be dynamic?
        {0x02a5f547, "FMSUB.D fa0, fa1, fa0, fa1, rne"},
        {0x02a5f54b, "FNMSUB.D fa0, fa1, fa0, fa1, rne"},
        {0x02a5f54f, "FNMADD.D fa0, fa1, fa0, fa1, rne"},
        {0x02a50553, "FADD.D fa0, fa1, fa0, rne"},
        {0x0aa50553, "FSUB.D fa0, fa1, fa0, rne"},
        {0x12a50553, "FMUL.D fa0, fa1, fa0, rne"},
        {0x1aa50553, "FDIV.D fa0, fa1, fa0, rne"},
        {0x5a051553, "FSQRT.D fa0, fa1, rne"},
        {0x22a50553, "FSGNJ.D fa0, fa1, fa0"},
        {0x22a51553, "FSGNJN.D fa0, fa1, fa0"},
        {0x22a52553, "FSGNJX.D fa0, fa1, fa0"},
        {0x2aa50553, "FMIN.D fa0, fa1, fa0"},
        {0x2aa51553, "FMAX.D fa0, fa1, fa0"},
        {0x40151553, "FCVT.S.D fa0, fa1, rne"},
        {0x42050553, "FCVT.D.S fa0, fa1, rne"},
        {0xa2a52553, "FEQ.D a0, fa1, fa0"},
        {0xa2a51553, "FLT.D a0, fa1, fa0"},
        {0xa2a50553, "FLE.D a0, fa1, fa0"},
        {0xe2051553, "FCLASS.D a0, fa1"},
        {0xc2050553, "FCVT.W.D a0, fa1, rne"},
        {0xc2150553, "FCVT.WU.D a0, fa1, rne"},
        {0xd2050553, "FCVT.D.W fa0, a1, rne"},
        {0xd2150553, "FCVT.D.WU fa0, a1, rne"},

        // RV64D instructions (in addition to RV32D)
        {0xc2250553, "FCVT.L.D a0, fa1, rne"},
        {0xc2350553, "FCVT.LU.D a0, fa1, rne"},
        {0xe2050553, "FMV.X.D a0, fa1"},
        {0xd2250553, "FCVT.D.L fa0, a1, rne"},
        {0xd2350553, "FCVT.D.LU fa0, a1, rne"},
        {0xf2050553, "FMV.D.X fa0, a0"},

        // RV32Q instructions
        {0x00004507, "FLQ fa0, 0(zero)"},
        {0x00a53027, "FSQ fa0, 0(a0)"},
        {0x06a5f543, "FMADD.Q fa0, fa1, fa0, fa1, rne"},
        {0x06a5f547, "FMSUB.Q fa0, fa1, fa0, fa1, rne"},
        {0x06a5f54b, "FNMSUB.Q fa0, fa1, fa0, fa1, rne"},
        {0x06a5f54f, "FNMADD.Q fa0, fa1, fa0, fa1, rne"},
        {0x06a50553, "FADD.Q fa0, fa1, fa0, rne"},
        {0x0ea50553, "FSUB.Q fa0, fa1, fa0, rne"},
        {0x16a50553, "FMUL.Q fa0, fa1, fa0, rne"},
        {0x1ea50553, "FDIV.Q fa0, fa1, fa0, rne"},
        {0x5e051553, "FSQRT.Q fa0, fa1, rne"},
        {0x26a50553, "FSGNJ.Q fa0, fa1, fa0"},
        {0x26a51553, "FSGNJN.Q fa0, fa1, fa0"},
        {0x26a52553, "FSGNJX.Q fa0, fa1, fa0"},
        {0x2ea50553, "FMIN.Q fa0, fa1, fa0"},
        {0x2ea51553, "FMAX.Q fa0, fa1, fa0"},
        {0x40351553, "FCVT.S.Q fa0, fa1, rne"},
        {0x46050553, "FCVT.Q.S fa0, fa1, rne"},
        {0x42351553, "FCVT.D.Q fa0, fa1, rne"},
        {0x46150553, "FCVT.Q.D fa0, fa1, rne"},
        {0xa6a52553, "FEQ.Q a0, fa1, fa0"},
        {0xa6a51553, "FLT.Q a0, fa1, fa0"},
        {0xa6a50553, "FLE.Q a0, fa1, fa0"},
        {0xe6051553, "FCLASS.Q a0, fa1"},
        {0xc6050553, "FCVT.W.Q a0, fa1, rne"},
        {0xc6150553, "FCVT.WU.Q a0, fa1, rne"},
        {0xd6050553, "FCVT.Q.W fa0, a1, rne"},
        {0xd6150553, "FCVT.Q.WU fa0, a1, rne"},

        // RV64Q instructions (in addition to RV32Q)
        {0xc6250553, "FCVT.L.Q a0, fa1, rne"},
        {0xc6350553, "FCVT.LU.Q a0, fa1, rne"},
        {0xd6250553, "FCVT.Q.L fa0, a1, rne"},
        {0xd6350553, "FCVT.Q.LU fa0, a1, rne"}

    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        printf("%pI\n", &tests[i].instruction);
    }

    uart_putstring("All disassemble tests passed!\n");
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
    test_disassemble();
    test_disassemble2();

    uart_putstring("Successfully finished executing main() in test_strings_printf.c\n");
}