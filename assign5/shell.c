/* File: shell.c
 * -------------
 * ***** Create shell *****
 */
#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "strings.h"
#include "mango.h"

#define LINE_LEN 80

// Module-level global variables for shell
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;


// NOTE TO STUDENTS:
// Your shell commands output various information and respond to user
// error with helpful messages. The specific wording and format of
// these messages would not generally be of great importance, but
// in order to streamline grading, we ask that you aim to match the
// output of the reference version.
//
// The behavior of the shell commands is documented in "shell_commands.h"
// https://cs107e.github.io/header#shell_commands
// The header file gives example output and error messages for all
// commands of the reference shell. Please match this wording and format.
//
// Your graders thank you in advance for taking this care!


static const command_t commands[] = {
    {"help",   "help [cmd]",         "print command usage and description", cmd_help},
    {"echo",   "echo [args]",        "print arguments", cmd_echo},
    {"clear",  "clear",              "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot",             "reboot the Mango Pi", cmd_reboot},
    {"peek",   "peek [addr]",        "print contents of memory at address", cmd_peek},
    {"poke",   "poke [addr] [val]",  "store value into memory at address", cmd_poke}
};

int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i)
        module.shell_printf("%s%s", argv[i], (i < argc - 1) ? " " : "\n");
    return 0;
}

int cmd_help(int argc, const char *argv[]) {
    if (argc > 2) {
        module.shell_printf("error: incorrect number of arguments to help\n");
        return 1;
    }
    
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (argc == 1 || strcmp(argv[1], commands[i].name) == 0) {
            module.shell_printf("%-16s %s\n", commands[i].usage, commands[i].description);
            if (argc == 2) return 0;
        }
    }
    
    if (argc == 2) {
        module.shell_printf("error: no such command '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}

int cmd_clear(int argc, const char* argv[]) {
    module.shell_printf("\f");
    return 0;
}

int cmd_reboot(int argc, const char *argv[]) {
    module.shell_printf("Rebooting...\n");
    mango_reboot();
    return 0;
}

static int poke_peek_helper(const char *arg, unsigned long *addr, const char *cmd) {
    const char *endptr;
    *addr = strtonum(arg, &endptr);
    
    if (*endptr != '\0') {
        module.shell_printf("error: %s cannot convert '%s'\n", cmd, arg);
        return 1;
    }
    if (*addr % 4 != 0) {
        module.shell_printf("error: %s address must be 4-byte aligned\n", cmd);
        return 1;
    }
    if (*addr >= 0x80000000) {
        module.shell_printf("error: %s address out of valid range\n", cmd);
        return 1;
    }
    return 0;
}

int cmd_peek(int argc, const char *argv[]) {
    if (argc != 2) {
        module.shell_printf("error: peek expects 1 argument [addr]\n");
        return 1;
    }

    unsigned long addr;
    if (poke_peek_helper(argv[1], &addr, "peek")) return 1;

    unsigned int val = *(unsigned int *)addr;
    module.shell_printf("0x%08x:  %08x\n", (unsigned int)addr, val);
    return 0;
}

int cmd_poke(int argc, const char *argv[]) {
    if (argc != 3) {
        module.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return 1;
    }

    unsigned long addr;
    if (poke_peek_helper(argv[1], &addr, "poke")) return 1;

    const char *endptr;
    unsigned long val = strtonum(argv[2], &endptr);
    if (*endptr != '\0') {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return 1;
    }

    *(unsigned int *)addr = val;
    return 0;
}

void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    module.shell_read = read_fn;
    module.shell_printf = print_fn;
}

void shell_bell(void) {
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize) {
    size_t temp = 0;
    
    while (1) {
        char ch = module.shell_read();
        if (ch > 0x7f) continue;
        
        if (ch == '\n' || ch == '\r') {
            module.shell_printf("\n");
            buf[temp] = '\0';
            return;
        }
        
        if (ch == '\b' && temp > 0) {
            temp--;
            module.shell_printf("\b \b");
        } else if (ch != '\b' && temp < bufsize - 1) {
            buf[temp++] = ch;
            module.shell_printf("%c", ch);
        } else {
            shell_bell();
        }
    }
}

int shell_evaluate(const char *line) {
    const char *tokens[LINE_LEN];
    int num_tokens = 0;
    
    while (*line) {
        while (*line == ' ' || *line == '\t' || *line == '\n') line++;
        if (!*line) break;
        
        tokens[num_tokens++] = line;
        while (*line && *line != ' ' && *line != '\t' && *line != '\n') line++;
        if (*line) *(char *)line++ = '\0';
    }
    
    if (!num_tokens) return 0;
    
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (strcmp(tokens[0], commands[i].name) == 0) {
            return commands[i].fn(num_tokens, tokens);
        }
    }
    
    module.shell_printf("error: no such command '%s'\n", tokens[0]);
    return 1;
}

void shell_run(void) {
    char line[LINE_LEN];
    module.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    while (1) {
        module.shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
