# Makefile for basic OS

# Compiler and tools
ASM = nasm
CC = gcc
LD = ld

# Directories
SRC_DIR = src

# Flags
ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -I$(SRC_DIR)
LDFLAGS = -m elf_i386 -T linker.ld

# Source files
ASM_SOURCES = $(SRC_DIR)/kernel_entry.asm $(SRC_DIR)/context_switch.asm
C_SOURCES = $(SRC_DIR)/kernel.c $(SRC_DIR)/memory.c $(SRC_DIR)/process.c $(SRC_DIR)/scheduler.c

# Object files
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)

# Output files
KERNEL_BIN = kernel.bin
BOOT_BIN = boot.bin
OS_IMAGE = os-image.bin

.PHONY: all clean run

all: $(OS_IMAGE)

# Build boot sector
$(BOOT_BIN): $(SRC_DIR)/boot.asm $(SRC_DIR)/print.asm $(SRC_DIR)/disk.asm $(SRC_DIR)/gdt.asm $(SRC_DIR)/switch_pm.asm
	$(ASM) -f bin $(SRC_DIR)/boot.asm -o $(BOOT_BIN)

# Compile assembly files
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ASM_OBJECTS) $(C_OBJECTS)

# Create OS image (boot sector + kernel)
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(OS_IMAGE)
	# Pad to ensure kernel is loaded correctly
	truncate -s 64K $(OS_IMAGE)

# Run in QEMU
run: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

# Clean build files
clean:
	rm -f *.o $(SRC_DIR)/*.o *.bin $(OS_IMAGE)
