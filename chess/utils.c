#include "utils.h"
#include "uart.h"

int abs(int x) {
    return x >= 0 ? x : -x;
}

char tolower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void itoa(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num /= 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    for (int start = 0, end = i - 1; start < end; start++, end--) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
    }
}

void uart_getstring(char *buffer, int length) {
    int i = 0;
    char c;
    while (i < length - 1) {
        c = uart_getchar();

        if (c == '\b' || c == 127) {  // Handle backspace
            if (i > 0) {
                uart_putstring("\b \b");
                i--;
            }
            continue;
        }

        if (c == '\n' || c == '\r') {  // Handle enter/return
            uart_putchar('\n');
            break;
        }

        if (i < length - 1) {  // Handle regular characters
            uart_putchar(c);
            buffer[i++] = c;
        }
    }
    buffer[i] = '\0';
}