#include <stdio.h>
#include <string.h>

void trimspace(char *string) {
	int i = 0, j = 0;

	while (string[i] != '\0') {
		if (string[i] != ' ') {
			string[j++] = string[i];
		}
		i++;
	}
	string[j] = '\0';
}


void parser(char *string) {
	trimspace(string);
}

int main() {
	char must_parse[] = "int x;";
	parser(must_parse);
	printf("Parsed '%s'\n", must_parse);
	return 0;
}
