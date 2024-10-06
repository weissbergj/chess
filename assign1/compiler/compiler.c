// This file translates the C in input.s into assembly file assembly.s to be assembled into binary script.binary
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> //for isspace()
//#include "imports.h"



void pb_output(int value, FILE *output_file);
void pb_on(int value, FILE *output_file);
void custom_delay(int value, FILE *output_file);
//void big_delay TODO

char *trimwhitespace(char *str);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Error: argc !=3\n");
    return 1;
  }

  FILE *input_file =  fopen(argv[1], "r");   // 0 is compiler.c; 1 is assembly.s
  FILE *output_file =  fopen(argv[2], "w");  // 2 is script.s
 
  if (!input_file || !output_file) {
    fprintf(stderr, "Error opening input or output file.\n");
    return 1;
  }

  char line[256];
  while (fgets(line, sizeof(line), input_file)) {
    printf("%s", line);  // Print each line read for debugging
    char *trimmed_line = trimwhitespace(line);  // Trim spaces

    int value;
    int initial_state;
    int increment;
    int strong;
    int weak;

    if (sscanf(trimmed_line, "pb_output(%d)", &value) == 1) {
      pb_output(value, output_file);
    } else if (sscanf(trimmed_line, "pb_on(%d)", &value) == 1) {
        pb_on(value, output_file);
    } else if (sscanf(trimmed_line, "custom_delay(%d)", &value) == 1) {
        custom_delay(value, output_file);
    } else {
        printf("Unrecognized line: %s\n", trimmed_line); // debugging
    }

  }
  
  fclose(input_file);
  fclose(output_file);
 
  return 0;
}

char *trimwhitespace(char *str) {
  while(isspace((unsigned char)*str)) str++;
  return str;
}

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

int delay_counter = 0;

void custom_delay(int value, FILE *output_file) {
  fprintf(output_file, "lui t0, 0xFFFF\n");
  fprintf(output_file, "custom_delay_%d:\n", delay_counter);
  fprintf(output_file, "addi t0, t0, -%d\n", value);
  fprintf(output_file, "bge t0, zero, custom_delay_%d\n", delay_counter);
  delay_counter++;
}

//void big_delay() { TODO


