NAME	= kfs.bin
ISO	= kfs.iso

CC	= gcc
AS	= nasm
LD	= ld
GRUB	= grub-mkrescue

CFLAGS	= -fno-builtin -fno-exceptions -fno-stack-protector -fno-rtti \
	-nostdlib -nodefaultlibs -ffreestanding -fno-pie -fno-pic -I include
ASFLAGS	= -f elf32
LDFLAGS	= -m elf_i386 -nostdlib -T linker.ld

ifeq ($(shell uname), Darwin)
CC	= i686-elf-gcc
LD	= i686-elf-ld
GRUB	= i686-elf-grub-mkrescue
else
CFLAGS	+= -m32
endif

SRCS	= $(wildcard src/*.c)
ASMS	= $(wildcard src/*.s)
OBJS	= $(SRCS:.c=.o) $(ASMS:.s=.o)

.PHONY: all iso run clean fclean re

all: $(NAME)

$(NAME): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

iso: $(NAME)
	mkdir -p iso/boot/grub
	cp $(NAME) iso/boot/kfs.bin
	$(GRUB) --compress=xz --fonts= --locales= --themes= \
	--install-modules="multiboot biosdisk iso9660 normal configfile reboot halt" \
	-o $(ISO) iso

run: iso
	qemu-system-i386 -boot d -cdrom $(ISO)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME) $(ISO) iso/boot/kfs.bin

re: fclean all
