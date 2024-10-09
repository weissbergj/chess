compiler: types_operators.c
	gcc types_operators.c -o compiler

assembly.s: compiler input.txt
	./compiler input.txt assembly.s

assembly.bin: assembly.s
	riscv64-unknown-elf-as assembly.s -o assembly.o
	riscv64-unknown-elf-objcopy assembly.o -O binary assembly.bin

run: assembly.bin
	mango-run assembly.bin
