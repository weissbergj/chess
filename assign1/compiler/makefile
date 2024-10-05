compiler: compiler.c
	gcc compiler.c -o compiler

script.s: compiler
	./compiler output 3

script.bin: script.s
	riscv64-unknown-elf-as script.s -o script.o
	riscv64-unknown-elf-objcopy script.o -O binary script.bin

run: script.bin
	mango-run script.bin

