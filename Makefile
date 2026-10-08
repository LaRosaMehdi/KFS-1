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

# Local GRUB i386-pc modules (no sudo). Populated by `make deps`.
LOCAL_GRUB	= .deps/grub/i386-pc
GRUB_PC_VER	:= $(shell apt-cache show grub-pc-bin 2>/dev/null | awk '/^Version:/{print $$2; exit}')
ifeq ($(GRUB_PC_VER),)
GRUB_PC_VER	= 2.06-2ubuntu7.2
endif
GRUB_PC_DEB_URL	?= http://archive.ubuntu.com/ubuntu/pool/main/g/grub2/grub-pc-bin_$(GRUB_PC_VER)_amd64.deb

ifeq ($(shell uname), Darwin)
CC	= i686-elf-gcc
LD	= i686-elf-ld
GRUB	= i686-elf-grub-mkrescue
else
CFLAGS	+= -m32
SYSTEM_GRUB_IMG	:= $(shell find /usr/lib64/grub /usr/lib/grub -name boot_hybrid.img 2>/dev/null | head -1)
ifneq ($(SYSTEM_GRUB_IMG),)
GRUB_DIR	:= $(dir $(SYSTEM_GRUB_IMG))
else ifneq ($(wildcard $(LOCAL_GRUB)/boot_hybrid.img),)
GRUB_DIR	:= $(LOCAL_GRUB)
endif
ifneq ($(GRUB_DIR),)
GRUBFLAGS	+= -d $(GRUB_DIR)
endif
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

.PHONY: all bonus iso run run-bonus test deps clean fclean re

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

# Download grub-pc-bin modules into .deps/ (no root required).
deps:
	@command -v curl >/dev/null || { echo "curl is required for make deps"; exit 1; }
	@command -v dpkg-deb >/dev/null || { echo "dpkg-deb is required for make deps"; exit 1; }
	mkdir -p .deps
	curl -fsSL -o .deps/grub-pc-bin.deb "$(GRUB_PC_DEB_URL)"
	rm -rf .deps/grub .deps/extract
	mkdir -p .deps/extract .deps/grub
	dpkg-deb -x .deps/grub-pc-bin.deb .deps/extract
	cp -a .deps/extract/usr/lib/grub/i386-pc .deps/grub/
	rm -rf .deps/extract .deps/grub-pc-bin.deb
	test -f $(LOCAL_GRUB)/boot_hybrid.img
	@echo "ok: GRUB modules in $(LOCAL_GRUB)"

iso: $(NAME)
ifeq ($(shell uname), Linux)
ifeq ($(GRUB_DIR),)
	$(MAKE) deps
	$(MAKE) iso
else
	cp $(NAME) iso/boot/kfs.bin
	$(GRUB) $(GRUBFLAGS) -o $(ISO) iso
endif
else
	cp $(NAME) iso/boot/kfs.bin
	$(GRUB) $(GRUBFLAGS) -o $(ISO) iso
endif

run: iso
	qemu-system-i386 -boot d -cdrom $(ISO)

test: iso
	python3 scripts/check_boot.py --bin $(NAME) --iso $(ISO)

clean:
	rm -rf build

fclean: clean
	rm -f kfs.bin kfs.iso kfs_bonus.bin kfs_bonus.iso iso/boot/kfs.bin

re: fclean all
