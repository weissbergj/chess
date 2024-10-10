NAME = blink_actled

ARCH = -march=rv64im -mabi=lp64
CFLAGS = $(ARCH) -g -Og -Wall -ffreestanding
LDFLAGS = -nostdlib -e main

all: $(NAME).bin

%.o: blink_actled.c
	riscv64-unknown-elf-gcc $(CFLAGS) -c $< -o $@

%.elf: %.o
	riscv64-unknown-elf-gcc $(LDFLAGS) $< -o $@

%.bin: %.elf
	riscv64-unknown-elf-objcopy $< -O binary $@

run: %.bin
	mango-run %.bin

clean:
	rm -f *.o *.elf *.bin

.PRECIOUS: %.o %.elf
