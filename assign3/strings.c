/* File: strings.c
 * ---------------
 * ***** TODO: add your file header comment here *****
 */
#include "strings.h"

void *memcpy(void *dst, const void *src, size_t n) {
    /* Copy contents from src to dst one byte at a time */
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void *memset(void *dst, int val, size_t n) {
    /***** TODO: Your code goes here *****/
    return NULL;
}

size_t strlen(const char *str) {
    /* Implementation a gift to you from lab3 */
    size_t n = 0;
    while (str[n] != '\0') {
        n++;
    }
    return n;
}

int strcmp(const char *s1, const char *s2) {
	int i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0') {
		i++;
	}
	return ((int)s1[i] - (int)s2[i]);
}

size_t strlcat(char *dst, const char *src, size_t dstsize) {
	size_t i = 0;
	size_t dst_len = strlen(dst);
	size_t src_len = strlen(src);

	if (dst_len >= dstsize) {         //ensure room to append one char
		return dstsize + src_len;
	}

	while (src[i] != '\0' && dst_len + src_len < dstsize - 1) {
		dst[dst_len + i] = src[i];
		i++;
	}
	dst[dst_len + i] = '\0';
    	return dst_len + src_len;
}

unsigned long strtonum(const char *str, const char **endptr) {
	unsigned long i = 0;
	unsigned long num = 0;

	while (str[i] == ' ') {
		i++;
	}

	while (str[i] >= '0' && str[i] <= '9') {
		num *= 10 + str[i] - '0';
		i++;
	}

	if (endptr != NULL) {
		*endptr = &str[i];
	}

	return num;

}
