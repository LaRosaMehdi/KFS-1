NAME	= kfs.bin
ISO	= kfs.iso

CC	= gcc
AS	= nasm
LD	= ld
GRUB	= grub-mkrescue

CFLAGS	= -fno-builtin -fno-exceptions -fno-stack-protector \
	-nostdlib -nodefaultlibs -ffreestanding -fno-pie -fno-pic -I include
ASFLAGS	= -f elf32
LDFLAGS	= -m elf_i386 -nostdlib -T linker.ld
GRUBFLAGS	= --compress=xz --fonts= --locales= --themes= \
	--install-modules="multiboot biosdisk iso9660 normal configfile reboot halt"

ifeq ($(shell uname), Darwin)
CC	= i686-elf-gcc
LD	= i686-elf-ld
GRUB	= i686-elf-grub-mkrescue
else
CFLAGS	+= -m32
GRUBFLAGS	+= -d $(dir $(shell find /usr/lib64/grub /usr/lib/grub -name boot_hybrid.img 2>/dev/null | head -1))
endif

# bonus/ only holds files added or overridden on top of src/ and include/.
BUILD	= build
SRCS	= $(wildcard src/*.c src/*.s)
ifdef BONUS
NAME	= kfs_bonus.bin
ISO	= kfs_bonus.iso
BUILD	= build/bonus
CFLAGS	:= -I bonus/include $(CFLAGS)
SRCS	:= $(wildcard bonus/src/*.c bonus/src/*.s) \
	$(filter-out $(addprefix src/,$(notdir $(wildcard bonus/src/*))),$(SRCS))
endif
OBJS	= $(addprefix $(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))

.PHONY: all bonus iso run run-bonus clean fclean re

all: $(NAME)

bonus:
	$(MAKE) BONUS=1

run-bonus:
	$(MAKE) BONUS=1 run

$(NAME): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

iso: $(NAME)
	cp $(NAME) iso/boot/kfs.bin
	$(GRUB) $(GRUBFLAGS) -o $(ISO) iso

run: iso
	qemu-system-i386 -boot d -cdrom $(ISO)

clean:
	rm -rf build

fclean: clean
	rm -f kfs.bin kfs.iso kfs_bonus.bin kfs_bonus.iso iso/boot/kfs.bin

re: fclean all
