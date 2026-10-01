/*
** Minimal shell. Characters come from the keyboard interrupt, so a command
** runs inside that interrupt: no key is read until it returns.
*/

#include "shell.h"
#include "gdt.h"
#include "idt.h"
#include "io.h"
#include "klib.h"
#include "printk.h"
#include "tty.h"

# define SHELL_PROMPT       "kfs> "
/* Longest line, final '\0' included. Prompt + line must fit on one screen
** row: the line editing below never wraps. */
# define SHELL_LINE_SIZE    64
# define SHELL_NAME_WIDTH   8       /* column of the descriptions in `help` */
# define KBD_CONTROLLER     0x64    /* PS/2 controller status / command port */
# define KBD_INPUT_FULL     0x02    /* status bit: controller still busy */
# define KBD_RESET_CPU      0xFE    /* command: pulse the CPU reset line */

struct shell_command
{
    const char *name;
    const char *description;
    void (*run)(void);
};

struct shell_line
{
    char text[SHELL_LINE_SIZE];
    size_t length;
    size_t cursor;              /* index in `text` where the next key goes */
};

static struct shell_line g_lines[TTY_COUNT];

static void command_help(void);

static void halt_forever(void)
{
    while (1)
        __asm__ volatile ("cli; hlt");
}

static void command_halt(void)
{
    printk("System halted.\n");
    halt_forever();
}

static void command_reboot(void)
{
    struct idt_ptr empty_idt;

    while (inb(KBD_CONTROLLER) & KBD_INPUT_FULL)
        ;
    outb(KBD_CONTROLLER, KBD_RESET_CPU);
    /* Still running: an interrupt with no IDT triple-faults, which also
    ** resets the CPU. */
    empty_idt.limit = 0;
    empty_idt.base = 0;
    __asm__ volatile ("lidt %0; int $3" : : "m"(empty_idt));
    halt_forever();
}

/* Invalid opcode: goes through the real exception path (isr_handler), which
** dumps the stack, prints the panic message and halts. */
static void command_panic(void)
{
    __asm__ volatile ("ud2");
}

static const struct shell_command g_commands[] = {
    {"help", "list the commands", command_help},
    {"stack", "dump the kernel stack", print_k_stack},
    {"gdt", "print the GDT entries", gdt_print},
    {"clear", "clear this console and its history", tty_clear},
    {"halt", "stop the CPU", command_halt},
    {"reboot", "restart the machine", command_reboot},
    {"panic", "trigger a kernel panic", command_panic},
};

# define SHELL_COMMAND_COUNT (sizeof(g_commands) / sizeof(g_commands[0]))

static void command_help(void)
{
    size_t command_index;
    size_t column;

    command_index = 0;
    while (command_index < SHELL_COMMAND_COUNT)
    {
        tty_write(g_commands[command_index].name);
        column = strlen(g_commands[command_index].name);
        while (column++ < SHELL_NAME_WIDTH)
            tty_putchar(' ');
        printk("%s\n", g_commands[command_index].description);
        command_index++;
    }
}

/* Runs `line` once the surrounding spaces are stripped; an empty line does
** nothing. */
static void run_line(char *line)
{
    size_t length;
    size_t command_index;

    while (*line == ' ')
        line++;
    length = strlen(line);
    while (length > 0 && line[length - 1] == ' ')
        line[--length] = '\0';
    if (length == 0)
        return ;
    command_index = 0;
    while (command_index < SHELL_COMMAND_COUNT)
    {
        if (strcmp(line, g_commands[command_index].name) == 0)
        {
            g_commands[command_index].run();
            return ;
        }
        command_index++;
    }
    printk("unknown command: %s (try help)\n", line);
}

/* Redraws `line` from its cursor to its end, plus one blank that erases the
** cell left over by a deletion, then puts the screen cursor back. */
static void redraw_after_cursor(const struct shell_line *line)
{
    size_t index;

    index = line->cursor;
    while (index < line->length)
        tty_putchar(line->text[index++]);
    tty_putchar(' ');
    tty_move_cursor(0, -(int)(line->length - line->cursor + 1));
}

void shell_prompt(void)
{
    tty_write(SHELL_PROMPT);
}

void shell_move_cursor(int column_delta)
{
    struct shell_line *line;

    line = &g_lines[tty_current()];
    if ((column_delta < 0 && line->cursor == 0)
        || (column_delta > 0 && line->cursor == line->length))
        return ;
    line->cursor += column_delta;
    tty_move_cursor(0, column_delta);
}

void shell_handle_char(char c)
{
    struct shell_line *line;

    line = &g_lines[tty_current()];
    if (c == '\n')
    {
        tty_putchar('\n');
        line->text[line->length] = '\0';
        line->length = 0;
        line->cursor = 0;
        run_line(line->text);
        shell_prompt();
    }
    else if (c == '\b')
    {
        if (line->cursor == 0)
            return ;
        memmove(line->text + line->cursor - 1, line->text + line->cursor,
            line->length - line->cursor);
        line->length--;
        line->cursor--;
        tty_move_cursor(0, -1);
        redraw_after_cursor(line);
    }
    else if (c >= ' ' && c <= '~' && line->length < SHELL_LINE_SIZE - 1)
    {
        memmove(line->text + line->cursor + 1, line->text + line->cursor,
            line->length - line->cursor);
        line->text[line->cursor] = c;
        line->length++;
        redraw_after_cursor(line);
        shell_move_cursor(1);
    }
}
