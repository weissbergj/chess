// This file containes type declarations and operations like =-+<>++%*/

declare_int(int value, FILE *output_file);
multiply_int
divide_int
remainder_int


//operations function that does all of those after i confirm they each work...includes parentheses/PEMDAS
// REMEMBER: parse for all ints that aren't in functions and then do .data. then .text; ignore local declarations and add them to stack
// also might have to add things like t0 output from add to stack 
// make sure to parse for all things like +=, -=, *=, /=

// int x, y, z;
void declare_int(char letter[], FILE *ouptut_file) {
  for (int i = 0; letter[i] != "\0"; i ++) {  
    fprintf(output_file, "%c: .zero  4\n", letter[i]);
  }
}

//int x = 1;
void declare_int_equal(char letter, int value, FILE *output_file) {
  fprintf(output_file, "%c: .word %d\n", letter, value);
}

// a + b;
void add_int(int a, int b, FILE *output_file) {
  fprintf(output_file, "li t0, %d\n", a);
  fprintf(output_file, "addi t0, t0, %d\n", b)
}

// a * b; THIS MAY BREAK IF MUL DOES NOT EXIST; SAME WITH DIV
void multiply_int(int a, int b, FILE *output_file) {
  load_into_register(a, b, output_file);
  fprintf(output_file, "mul t2, t0, t1\n");
}

// a / b
void divide_int(int a, int b, FILE *ouptut_file) {
  load_into_register(a, b, output_file);
  fprintf(output_file, "div t2, t0, t1\n");
}

// a ^ b
void exponentiate_int(int a, int b, FILE *output_file) {
  fprintf(output_file, "li t0, %d\n", a);
  fprintf(output_file, "li t1, t0");
  for (i = 0; i <  b; i ++) {
    fprintf(output_file, "mul t0, t0, t1");
  }
}

//void modulo_int(int a, int b, FILE *output_file) {
//void shift_left_int(int a, int b, FILE *output_file) {
//void shift_right_int

// call this for bne, bge, blt, beq in place of *operation
void branch(const char *operation, int a, int b, const char *true_label, FILE *output_file) {
  load_into_register(a, b, output_file);
  fprintf(output_file, "%s t0, t1, %s\n", operation, true_label);
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

void boolean_logic(int *conditions, const char **operations, const char *operators, int num_conditions)


void if_else_(const char *operators[], const char *labels[])

void if_else_boolean
