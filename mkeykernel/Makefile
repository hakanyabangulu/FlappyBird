# Derleme araçları
CC = gcc
AS = nasm
LD = ld

# Derleme bayrakları
CFLAGS = -m32 -ffreestanding -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T link.ld

# Hedef dosyalar
OBJECTS = kernel_asm.o kernel.o

# Varsayılan hedef
all: kernel.bin

# kernel.asm derleme
kernel_asm.o: kernel.asm
	$(AS) $(ASFLAGS) kernel.asm -o kernel_asm.o

# kernel.c derleme
kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

# Bağlama
kernel.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJECTS)

# Temizleme
clean:
	rm -f *.o kernel.bin

# Çalıştırma
run: kernel.bin
	qemu-system-i386 -kernel kernel.bin -k en-us -device i8042 -vga std