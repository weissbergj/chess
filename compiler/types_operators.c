// This file containes type declarations and operations like =-+<>++%*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> //for isspace()
		   
void pb_output(int value, FILE *output_file);
void pb_on(int value, FILE *output_file);

void declaration_parse(char *trimmed_line, FILE *output_file);
void declaration_equal_parse(char *trimmed_line,  FILE *output_file);

void declare_int(char letter, FILE *output_file);
void declare_int_equal(char letter, int value, FILE *output_file);
void add_int(int a, int b, FILE *output_file);
void multiply_int(int a, int b, FILE *output_file);
void divide_int(int a, int b, FILE *output_file);
void exponentiate_int(int a, int b, FILE *output_file);
void branch(const char *operation, int a, int b, const char *true_label, FILE *output_file);
void while_int(const char *operation, int a, int b, const char *true_label, const char *function, FILE *output_file);
void load_into_register(int a, int b, FILE *output_file);
void post_increment(int a, int b, FILE *output_file);


//operations function that does all of those after i confirm they each work...includes parentheses/PEMDAS
// REMEMBER: parse for all ints that aren't in functions and then do .data. then .text; ignore local declarations and add them to stack
// also might have to add things like t0 output from add to stack 
// make sure to parse for all things like +=, -=, *=, /=

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Error: argc !=3\n");
		return 1;
	}

	FILE *input_file =  fopen(argv[1], "r");   
	FILE *output_file =  fopen(argv[2], "w");

	if (!input_file || !output_file) {
		fprintf(stderr, "Error opening input or output file.\n");
		return 1;
	}

	char line[256];
	while (fgets(line, sizeof(line), input_file)) {
		char *trimmed_line = line;
		printf("%s", line);

		declaration_parse(trimmed_line, output_file);
		declaration_equal_parse(trimmed_line, output_file);
	}
	
	fclose(input_file);
	fclose(output_file);
	return 0;
}

char *trimwhitespace(char *str) {
	while(isspace((unsigned char)*str)) str++;
	return str;
}


//FUTURE DECLARE FUNCTION EXAMPLE
// int x, y, z; in future do letter[] and loop
//void declare_int(char letter[], FILE *output_file) {
//	for (int i = 0; letter[i] != '\0'; i ++) {
//		fprintf(output_file, "%c: .zero  4\n", letter[i]);
//	}
//}

void declare_int(char letter, FILE *output_file) {
	fprintf(output_file, "%c: .zero 4\n", letter);
}

void declaration_parse(char *trimmed_line, FILE *output_file) {
	char letter;
	if (sscanf(trimmed_line, "int %c;", &letter) == 1) {
		declare_int(letter, output_file);
		}
}

//int x = 1;
void declare_int_equal(char letter, int value, FILE *output_file) {
	fprintf(output_file, "%c: .word %d\n", letter, value);
}

void declaration_equal_parse(char *trimmed_line,  FILE *output_file) {
	char letter;
	int value;
	if (sscanf(trimmed_line, "int %c = %d;", &letter, &value) == 2) {
		declare_int_equal(letter, value, output_file);
	}
}


// a + b;
void add_int(int a, int b, FILE *output_file) {
	fprintf(output_file, "li t0, %d\n", a);
	fprintf(output_file, "addi t0, t0, %d\n", b);
}

// a * b; THIS MAY BREAK IF MUL DOES NOT EXIST; SAME WITH DIV
void multiply_int(int a, int b, FILE *output_file) {
	load_into_register(a, b, output_file);
	fprintf(output_file, "mul t2, t0, t1\n");
}

// a / b / may have to fix and do add many times...
void divide_int(int a, int b, FILE *output_file) {
	load_into_register(a, b, output_file);
	fprintf(output_file, "div t2, t0, t1\n");
}

// a ^ b
void exponentiate_int(int a, int b, FILE *output_file) {
	fprintf(output_file, "li t0, %d\n", a);
	fprintf(output_file, "li t1, t0\n");
	for (int i = 0; i <  b; i ++) {
		fprintf(output_file, "mul t0, t0, t1\n");
	}
}

//int branch_counter = 0
// call this for bne, bge, blt, beq in place of *operation
void branch(const char *operation, int a, int b, const char *true_label, FILE *output_file) {
	load_into_register(a, b, output_file);
	fprintf(output_file, "%s t0, t1, %s\n", operation, true_label);
}

//move this to structs
int while_counter = 0;
void while_int(const char *operation, int a, int b, const char *true_label, const char *function, FILE *output_file) {
	char buffer0[50];
	char buffer1[50];

	sprintf(buffer0, "check_condition%d", while_counter);
	sprintf(buffer1, "while_true%d", while_counter);

	fprintf(output_file, "j %s\n", buffer0);
	fprintf(output_file, "%s\n", buffer1);
  
	fprintf(output_file, "%s\n", function);

	fprintf(output_file, "%s:\n", buffer0);
  
	branch(operation, a, b, buffer1, output_file); 
}

void load_into_register(int a, int b, FILE *output_file) {
  fprintf(output_file, "li t0, %d\n", a);
  fprintf(output_file, "li t1, %d\n", b);
}

// int b is + or -
void post_increment(int a, int b, FILE *output_file) {
  fprintf(output_file, "li t0, %d\n", a);
  fprintf(output_file, "addi t0, t0, %d\n", b);
}

void print_assembly(const char *text,FILE *output_file) {
	fprintf(output_file, "%s\n", text);
}






//FOR DEBUGGING:
void pb_output(int value, FILE *output_file) {
    unsigned int base = 0x2000;
    unsigned int a1 = 0x00000001;
    char a1_str[32];

    if (value <= 0) {
        fprintf(output_file, "lui a0, 0x%04x\n", base);
        fprintf(output_file, "li a1, 0x0\n");
        fprintf(output_file, "sw a1, 0x30(a0)\n");
    } else {
        a1 = value;
        sprintf(a1_str, "%d", value);
        fprintf(output_file, "lui a0, 0x%04x\n", base);
        fprintf(output_file, "li a1, 0x%s\n", a1_str);
        fprintf(output_file, "sw a1, 0x30(a0)\n");
        return;
    }
  
}

void pb_on(int value, FILE *output_file) {
    unsigned int base = 0x2000;
    unsigned int a1 = 0b00000001;
    char a1_str[32];  

    if (value <= 0) {
        a1 = 0;
    } else {
        a1 = value;
    }
  
    pb_output(value, output_file);
    fprintf(output_file, "lui a0, 0x%04x\n", base);
    fprintf(output_file, "li a1, 0b%d\n", a1);
    fprintf(output_file, "sw a1, 0x40(a0)\n");
}



//if x < y, else no elif, no &&
//void if_else(const char *integers[]; const char *operators[], const char *labels[], FILE *ouptut_file) {
//for (i = 0; i < 1; i++)




// NOT NECESSARY BELOW
// if (x) // do this in the future, for now --> if (x != 0)
// take care of NULL vs 0, NULL = ((void*)0)
// void modulo_int(int a, int b, FILE *output_file) {
// void shift_left_int(int a, int b, FILE *output_file) {
// void shift_right_int

