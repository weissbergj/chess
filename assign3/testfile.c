#include <string.h>
#include <stdio.h>

char *str = "abcd";
char *str2 = "abcdffff";


int main() {
	printf("this is it: %d", strcmp(str, str2));
	return 0;
}
