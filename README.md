# KFS-1 — Grub, boot and screen

Kernel i386 freestanding (42 Kernel From Scratch, partie mandatory).
GRUB charge le binaire, le code ASM pose le header Multiboot et appelle `main`,
qui affiche **42** en VGA texte (80x25, memoire `0xB8000`).

Aucune libc hote n'est liee. Image virtuelle : `kfs.iso` (bien sous 10 Mo).

Le dossier `bonus/` reprend le meme kernel avec scroll, curseur, couleurs,
`printk`, clavier et 4 ecrans (F1-F4).

## Lancer

```bash
make        # compile kfs.bin
make iso    # installe GRUB sur kfs.iso
make run    # QEMU graphique + menu GRUB (test manuel)
make test   # boot auto QEMU, verifie 42 a l'ecran (sans fenetre)
make fclean
```

Bonus, **depuis la racine** `KFS-1/` :

```bash
make -C bonus run
```

Bonus, **si tu es deja dans** `bonus/` :

```bash
make run
```

Sequence au `make run` :

1. QEMU boot le CD
2. Menu **GNU GRUB** (reste affiche, pas de timeout)
3. Fleches + Entree :
   - **Start KFS-1** — kernel, ecran `42`
   - **Reboot** — redemarre la VM
   - **Halt** — eteint la VM
4. Cliquer la fenetre QEMU pour le clavier

Dependances Linux 42 : `gcc` (-m32), `nasm`, `ld`, `grub-mkrescue`, `qemu-system-i386`.
Sur macOS : `i686-elf-gcc`, `i686-elf-ld`, `i686-elf-grub-mkrescue` (detectes par le Makefile).
`make test` a besoin de QEMU et de Python 3.

## CI (GitHub Actions)

Fichier : `.github/workflows/ci.yml`.

A chaque **push** et **pull request**, GitHub lance une VM **Ubuntu 24.04** (pas ton Mac).
Il n'y a pas d'ecran : on ne peut pas ouvrir la fenetre de `make run`.

Le job **Build and test** :

1. Installe gcc 32-bit, nasm, make, GRUB (`grub-pc-bin`), xorriso, python3
2. Installe **QEMU** (`qemu-system-x86`) — sans ca le test de boot echoue
3. `make test` — partie mandatory
4. `make -C bonus test` — bonus

`make test` lance `scripts/check_boot.py` :

1. Compile le kernel et construit `kfs.iso` (comme `make iso`)
2. Verifie le header **Multiboot** si `grub-file` est la
3. QEMU **sans fenetre** (`-display none`) :
   - boot direct `-kernel kfs.bin`
   - boot ISO + menu GRUB + Entree (comme `make run`)
4. Lit le tampon VGA `0xB8000` (le meme ecran 80x25 que la fenetre QEMU)
5. Le texte **`42`** doit y etre, sinon exit code != 0

`make run` ne change pas : QEMU graphique, menu GRUB a la main.

## Fichiers

| Fichier | Role |
|---|---|
| `src/boot.s` | Header Multiboot v1, pile, `_start` → `main` |
| `src/kernel.c` | `main` : init VGA, affiche 42, halt |
| `src/vga.c` | Interface ecran (buffer `0xB8000`) |
| `src/klib.c` | `strlen`, `strcmp`, memoire |
| `include/` | Types kernel, prototypes VGA / klib |
| `linker.ld` | Script linker perso (chargement a 1 MiB) |
| `iso/boot/grub/grub.cfg` | Menu GRUB |
| `Makefile` | Compile ASM+C, link, ISO, `test` |
| `scripts/check_boot.py` | Boot QEMU headless, check VGA `42` |
| `.github/workflows/ci.yml` | CI Ubuntu : deps + `make test` |

## Flags (sujet III.2.2)

`-fno-builtin -fno-exceptions -fno-stack-protector -fno-rtti -nostdlib -nodefaultlibs`

(`-fno-exception` du PDF n'existe pas dans gcc → `-fno-exceptions`.
`-fno-rtti` est un flag C++ : gcc previent, on le garde pour le sujet.)

Aussi : `-ffreestanding -fno-pie -fno-pic`, et `-m32` sous Linux.
Link : `ld -m elf_i386 -nostdlib -T linker.ld`.
