# KFS-1 — Grub, boot and screen

Freestanding i386 kernel (42 Kernel From Scratch, mandatory part).
GRUB loads the binary, the ASM code provides the Multiboot header and calls
`main`, which prints **42** in VGA text mode (80x25, memory at `0xB8000`).

No host libc is linked. Virtual image: `kfs.iso` (well under 10 MB).

The bonus adds scrolling, a cursor, colors, `printk`, keyboard input and 4
screens (F1-F4). `bonus/` only holds added or replaced files: a file in
`bonus/src/` replaces the one with the same name in `src/`, and
`bonus/include/` is searched before `include/`. Everything else (boot, linker
script, klib, GRUB) is shared.

## Running

```bash
make        # build kfs.bin
make iso    # install GRUB into kfs.iso
make run    # QEMU + GRUB menu
make test   # headless QEMU, checks that 42 is on screen
make fclean
```

Bonus (`kfs_bonus.bin` / `kfs_bonus.iso`):

```bash
make bonus          # build kfs_bonus.bin
make run-bonus      # QEMU on kfs_bonus.iso
make BONUS=1 test   # headless check of the bonus ISO
```

What happens on `make run`:

1. QEMU boots the CD
2. **GNU GRUB** menu (stays on screen, no timeout)
3. Arrow keys + Enter:
   - **Start KFS-1** — kernel, `42` on screen
   - **Reboot** — restarts the VM
   - **Halt** — powers off the VM
4. Click the QEMU window to give it the keyboard

Linux (42) dependencies: `gcc` (-m32), `nasm`, `ld`, `grub-mkrescue`, `qemu-system-i386`.
On macOS: `i686-elf-gcc`, `i686-elf-ld`, `i686-elf-grub-mkrescue` (detected by the Makefile).
`make test` also needs QEMU and Python 3.

## CI (GitHub Actions)

Workflow: `.github/workflows/ci.yml`.

On every push and pull request, GitHub runs an Ubuntu 24.04 VM. There is no
display, so `make run` cannot open a window.

The **Build and test** job:

1. Installs 32-bit gcc, nasm, make, GRUB (`grub-pc-bin`), xorriso, Python 3
2. Installs QEMU (`qemu-system-x86`)
3. `make test` for the mandatory kernel
4. `make BONUS=1 test` for the bonus

`make test` runs `scripts/check_boot.py`. It builds the ISO, boots QEMU with
`-display none` (direct `-kernel` boot, then the ISO through the GRUB menu),
reads the VGA buffer at `0xB8000`, and fails if **42** is not on screen.
`make run` stays the graphical QEMU session.

## Files

| File | Role |
|---|---|
| `src/boot.s` | Multiboot v1 header, stack, `_start` → `main` |
| `src/kernel.c` | `main`: init VGA, print 42, halt |
| `src/vga.c` | Screen output (buffer at `0xB8000`) |
| `src/klib.c` | `strlen`, `strcmp`, memory functions |
| `include/` | Kernel types, I/O ports, VGA / klib prototypes |
| `linker.ld` | Custom linker script (loaded at 1 MiB) |
| `iso/boot/grub/grub.cfg` | GRUB menu |
| `Makefile` | Builds ASM+C, links, makes the ISO (`make` or `make bonus`) |
| `scripts/check_boot.py` | Headless QEMU boot, checks VGA for `42` |
| `.github/workflows/ci.yml` | Ubuntu CI: dependencies + `make test` |

Bonus (`bonus/`, replaces or extends the files above):

| File | Role |
|---|---|
| `src/kernel.c` | `main`: init GDT, IDT, PIC, consoles, keyboard, then `sti` |
| `src/gdt.c` | Flat GDT at `0x00000800`: kernel code/data/stack (`0x08`/`0x10`/`0x18`), user code/data/stack (`0x20`/`0x28`/`0x30`) |
| `src/idt.c` | IDT: 32 exceptions (red message + halt), 16 IRQs |
| `src/cpu.s` | `lgdt`/`lidt`, interrupt stubs that call into C |
| `src/pic.c` | 8259 PIC: IRQs remapped to 32-47, only the keyboard gets through |
| `src/keyboard.c` | PS/2 scancodes → characters, Shift, F1-F4, arrows, PageUp/PageDown |
| `src/tty.c` | 4 consoles, tab bar, scrolling + history, cursor movement, protected lines |
| `src/printk.c` | `printk`: `%c %s %d %i %u %x %X %p %%`, zero-padded width (`%08x`), `print_k_stack` |
| `src/vga.c` | VGA cells and hardware cursor |
| `include/` | Prototypes, GDT/IDT structures, bonus `vga.h` |

## How it works

**Boot**: GRUB finds the Multiboot header (`boot.s`), loads the kernel at
1 MiB and jumps to `_start`, which sets up a 16 KiB stack and calls
`main(magic, mb_info)`.

**Display**: the text screen is an array of 80x25 16-bit cells at `0xB8000`
(character + color). The blinking cursor is the VGA card's own, moved through
ports `0x3D4`/`0x3D5`.

**Consoles (bonus)**: each console keeps its own copy of the screen in memory.
Writes go to the active console's copy, which is then copied into VGA memory.
Row 0 shows the tabs ` 1  2  3  4 `, with the active console highlighted.
Each console's header is protected (`tty_lock_lines`): neither backspace nor
scrolling can erase it.
Rows that scroll off the top go to a 100-row history per console:
PageUp/PageDown scroll the view through it, and typing jumps back to the live
screen. The arrow keys move the cursor inside the editable area, and the next
characters are written there.

**Keyboard (bonus)**: a key press raises IRQ1 → the PIC sends vector 33 → the
`irq1` stub (`cpu.s`) saves the registers → `irq_handler` → `keyboard_handler`
reads the scancode from port `0x60`, then writes the character or switches
console (F1-F4) → the PIC is acknowledged (`pic_eoi`).

## Flags (subject III.2.2)

`-fno-builtin -fno-exceptions -fno-stack-protector -nostdlib -nodefaultlibs`

Also: `-ffreestanding -fno-pie -fno-pic`, and `-m32` on Linux.
Link: `ld -m elf_i386 -nostdlib -T linker.ld`.
