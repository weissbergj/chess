/* File: shell.c
 * -------------
 * ***** Create shell; extension includes command history, ctrl keys (a, e, u), tab completion, and left/right arrow insertion/deletion *****
 */
#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "strings.h"
#include "mango.h"
#include "ps2_keys.h"
#include "keyboard.h"
#include "hstimer.h"
#include "symtab.h"
#include "malloc.h"
#include "timer.h"
#include "interrupts.h"

static void clear_line(void);

#define MAX_HISTORY_ENTRIES 10
#define MAX_LINE_LENGTH 80
#define TEXT_START 0x40000000    // Start of text section from memmap
#define TEXT_END   0x40100000    // Conservative end estimate
#define PROFILE_INTERVAL 1000    // 1ms interval

typedef struct {
    char buffer[MAX_LINE_LENGTH];
    size_t len;
} history_entry_t;

// Global variables for shell
static struct {
    formatted_fn_t shell_printf;
    
    // Command history
    history_entry_t cmd_history[MAX_HISTORY_ENTRIES];
    int history_size;        // number of entries
    int history_index;       // position when browsing
    
    // Current line state
    char input_buffer[MAX_LINE_LENGTH];
    size_t cursor_position;
    size_t input_length;
    
    // Command counter
    int cmd_num;

    // Add these new fields for profiling
    unsigned int *profile_counts;    // Array to store instruction counts
    bool profiling;                  // Whether profiling is currently active
    unsigned int profile_start_time; // When profiling started
    unsigned int num_samples;        // Total samples collected
    unsigned int debug_interrupt_count;  // Add this field
    bool interrupt_received;
    unsigned long last_pc;
} shell_state;


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


int cmd_history(int argc, const char *argv[]) {
    for (int i = shell_state.history_size - 1; i >= 0; i--) {
        shell_state.shell_printf("%3d  %s\n", shell_state.history_size - i, 
                          shell_state.cmd_history[i].buffer);
    }
    return 0;
}

static const command_t commands[] = {
    {"help",   "help [cmd]",         "print command usage and description", cmd_help},
    {"echo",   "echo [args]",        "print arguments", cmd_echo},
    {"clear",  "clear",              "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot",             "reboot the Mango Pi", cmd_reboot},
    {"peek",   "peek [addr]",        "print contents of memory at address", cmd_peek},
    {"poke",   "poke [addr] [val]",  "store value into memory at address", cmd_poke},
    {"history", "history",            "show command history", cmd_history}
    // {"profile", "profile [on|off]",   "control profiler operation", cmd_profile}
};

int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i)
        shell_state.shell_printf("%s%s", argv[i], (i < argc - 1) ? " " : "\n");
    return 0;
}

int cmd_help(int argc, const char *argv[]) {
    if (argc > 2) {
        shell_state.shell_printf("error: incorrect number of arguments to help\n");
        return 1;
    }
    
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (argc == 1 || strcmp(argv[1], commands[i].name) == 0) {
            shell_state.shell_printf("%-16s %s\n", commands[i].usage, commands[i].description);
            if (argc == 2) return 0;
        }
    }
    
    if (argc == 2) {
        shell_state.shell_printf("error: no such command '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}

int cmd_clear(int argc, const char* argv[]) {
    shell_state.shell_printf("\f");
    return 0;
}

int cmd_reboot(int argc, const char *argv[]) {
    shell_state.shell_printf("Rebooting...\n");
    mango_reboot();
    return 0;
}

static int poke_peek_helper(const char *arg, unsigned long *addr, const char *cmd) {
    const char *endptr;
    *addr = strtonum(arg, &endptr);
    
    if (*endptr != '\0') {
        shell_state.shell_printf("error: %s cannot convert '%s'\n", cmd, arg);
        return 1;
    }
    if (*addr % 4 != 0) {
        shell_state.shell_printf("error: %s address must be 4-byte aligned\n", cmd);
        return 1;
    }
    if (*addr >= 0x80000000) {
        shell_state.shell_printf("error: %s address out of valid range\n", cmd);
        return 1;
    }
    return 0;
}

int cmd_peek(int argc, const char *argv[]) {
    if (argc != 2) {
        shell_state.shell_printf("error: peek expects 1 argument [addr]\n");
        return 0;
    }

    unsigned long addr;
    if (poke_peek_helper(argv[1], &addr, "peek")) return 0;

    unsigned int val = *(unsigned int *)addr;
    shell_state.shell_printf("0x%08x:  %08x\n", (unsigned int)addr, val);
    return 0;
}

int cmd_poke(int argc, const char *argv[]) {
    if (argc != 3) {
        shell_state.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return 0;
    }

    unsigned long addr;
    if (poke_peek_helper(argv[1], &addr, "poke")) return 0;

    const char *endptr;
    unsigned long val = strtonum(argv[2], &endptr);
    if (*endptr != '\0') {
        shell_state.shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return 0;
    }

    *(unsigned int *)addr = val;
    return 0;
}

void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    shell_state.shell_printf = print_fn;
    shell_state.cmd_num = 1;  // Start at 1
}

void shell_bell(void) {
    uart_putchar('\a');
}

static void insert_char(char ch) {
    if (shell_state.input_length >= MAX_LINE_LENGTH - 1) {
        shell_bell();
        return;
    }
    
    for (size_t i = shell_state.input_length; i > shell_state.cursor_position; i--)
        shell_state.input_buffer[i] = shell_state.input_buffer[i-1];
    
    shell_state.input_buffer[shell_state.cursor_position] = ch;
    shell_state.input_length++;
    shell_state.cursor_position++;
    
    for (size_t i = shell_state.cursor_position - 1; i < shell_state.input_length; i++)
        shell_state.shell_printf("%c", shell_state.input_buffer[i]);
    
    for (size_t i = shell_state.cursor_position; i < shell_state.input_length; i++)
        shell_state.shell_printf("\b");
}

static void delete_char(void) {
    if (shell_state.cursor_position == 0) {
        shell_bell();
        return;
    }
    
    shell_state.shell_printf("\b");
    shell_state.cursor_position--;
    
    for (size_t i = shell_state.cursor_position; i < shell_state.input_length - 1; i++)
        shell_state.input_buffer[i] = shell_state.input_buffer[i + 1];
    shell_state.input_length--;
    
    for (size_t i = shell_state.cursor_position; i < shell_state.input_length; i++)
        shell_state.shell_printf("%c", shell_state.input_buffer[i]);
    shell_state.shell_printf(" ");
    
    for (size_t i = shell_state.cursor_position; i < shell_state.input_length + 1; i++)
        shell_state.shell_printf("\b");
}

static void add_to_history(const char *line) {
    if (strlen(line) == 0) return;
    
    for (int i = MAX_HISTORY_ENTRIES - 1; i > 0; i--) {
        memcpy(shell_state.cmd_history[i].buffer, shell_state.cmd_history[i-1].buffer,
               shell_state.cmd_history[i-1].len + 1);
        shell_state.cmd_history[i].len = shell_state.cmd_history[i-1].len;
    }
    
    memcpy(shell_state.cmd_history[0].buffer, line, strlen(line) + 1);
    shell_state.cmd_history[0].len = strlen(line);
    if (shell_state.history_size < MAX_HISTORY_ENTRIES) shell_state.history_size++;
}

static void show_history_entry(int index) {
    clear_line();
    memcpy(shell_state.input_buffer, shell_state.cmd_history[index].buffer, 
           shell_state.cmd_history[index].len + 1);
    shell_state.input_length = shell_state.cmd_history[index].len;
    shell_state.cursor_position = shell_state.input_length;
    shell_state.shell_printf("%s", shell_state.input_buffer);
}

static int find_matches(const char *prefix, const char *matches[], int max_matches) {
    int count = 0;
    size_t len = strlen(prefix);
    char temp[MAX_LINE_LENGTH];
    
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]) && count < max_matches; i++) {
        memcpy(temp, commands[i].name, len);
        temp[len] = '\0';
        if (strcmp(prefix, temp) == 0)
            matches[count++] = commands[i].name;
    }
    return count;
}

static void complete_command(void) {
    char prefix[MAX_LINE_LENGTH];
    size_t prefix_len = 0;
    
    for (size_t i = shell_state.cursor_position; i > 0 && shell_state.input_buffer[i-1] != ' '; i--)
        prefix_len++;
    
    memcpy(prefix, &shell_state.input_buffer[shell_state.cursor_position - prefix_len], prefix_len);
    prefix[prefix_len] = '\0';
    
    const char *matches[sizeof(commands)/sizeof(commands[0])];
    int match_count = find_matches(prefix, matches, sizeof(matches)/sizeof(matches[0]));
    
    if (match_count == 0) {
        shell_bell();
    } else if (match_count == 1) {
        const char *completion = matches[0] + prefix_len;
        while (*completion) insert_char(*completion++);
    } else {
        shell_state.shell_printf("\n");
        for (int i = 0; i < match_count; i++)
            shell_state.shell_printf("%s  ", matches[i]);
        shell_state.shell_printf("\nPi[%d]> %s", shell_state.cmd_num, shell_state.input_buffer);
        for (size_t i = shell_state.cursor_position; i < strlen(shell_state.input_buffer); i++)
            shell_state.shell_printf("\b");
    }
}

static void move_cursor_to(size_t new_pos) {
    while (shell_state.cursor_position > new_pos) {
        shell_state.cursor_position--;
        shell_state.shell_printf("\b");
    }
    while (shell_state.cursor_position < new_pos) {
        shell_state.cursor_position++;
        shell_state.shell_printf("\x1b[C");
    }
}

static void clear_line(void) {
    move_cursor_to(0);
    shell_state.shell_printf("\x1b[K");  // Clear from cursor to end of line
    shell_state.input_length = 0;
    memset(shell_state.input_buffer, 0, MAX_LINE_LENGTH);
}

void shell_readline(char buf[], size_t bufsize) {
    shell_state.cursor_position = shell_state.input_length = shell_state.history_index = 0;
    memset(shell_state.input_buffer, 0, MAX_LINE_LENGTH);
    
    while (1) {
        key_event_t event = keyboard_read_event();
        if (event.action.what == KEY_RELEASE) continue; // process events not releases
        char ch = event.key.ch;
        
        // Ctrl keys
        if (event.modifiers & KEYBOARD_MOD_CTRL) {
            switch(ch) {
                case 'a':  // Ctrl-A: move to start
                    move_cursor_to(0);
                    continue;

                case 'e':  // Ctrl-E: move to end
                    move_cursor_to(shell_state.input_length);
                    continue;

                case 'u':  // Ctrl-U: clear line
                    clear_line();
                    continue;
            }
        }

        //  special keys
        switch(ch) {
            case PS2_KEY_ARROW_LEFT:
                if (shell_state.cursor_position > 0) {
                    shell_state.cursor_position--;
                    shell_state.shell_printf("\b");
                } else {
                    shell_bell();
                }
                continue;
                
            case PS2_KEY_ARROW_RIGHT:
                if (shell_state.cursor_position < shell_state.input_length) {
                    shell_state.cursor_position++;
                    shell_state.shell_printf("\x1b[C");
                } else {
                    shell_bell();
                }
                continue;

            case PS2_KEY_ARROW_UP:
                if (shell_state.history_index < shell_state.history_size) {
                    shell_state.history_index++;
                    show_history_entry(shell_state.history_index - 1);
                } else {
                    shell_bell();
                }
                continue;

            case PS2_KEY_ARROW_DOWN:
                if (shell_state.history_index > 0) {
                    shell_state.history_index--;
                    if (shell_state.history_index == 0) {
                        clear_line();
                    } else {
                        show_history_entry(shell_state.history_index - 1);
                    }
                } else {
                    shell_bell();
                }
                continue;
        }

        // tabs
        if (ch == '\t') {
            complete_command();
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            shell_state.shell_printf("\n");
            shell_state.input_buffer[shell_state.input_length] = '\0';
            memcpy(buf, shell_state.input_buffer, shell_state.input_length + 1);
            if (shell_state.input_length > 0) {
                add_to_history(buf);
            }
            return;
        }
        
        if (ch == '\b') {
            delete_char();
            continue;
        }
        
        if (ch >= ' ' && ch < 0x7f && shell_state.input_length < bufsize - 1) {
            insert_char(ch);
        }
    }
}

int shell_evaluate(const char *line) {
    const char *tokens[MAX_LINE_LENGTH];
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
    
    shell_state.shell_printf("error: no such command '%s'\n", tokens[0]);
    return 1;
}

void shell_run(void) {
    char line[MAX_LINE_LENGTH];
    
    shell_state.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    
    while (1) {
        shell_state.shell_printf("Pi[%d]> ", shell_state.cmd_num);
        shell_readline(line, sizeof(line));
        if (strlen(line) > 0) {  // Only increment for non-empty commands
            shell_state.cmd_num++;
        }
        shell_evaluate(line);
    }
}