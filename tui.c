#include "tinyctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define KEY_UP 1000
#define KEY_DOWN 1001

static struct termios tui_saved;
static int tui_raw;

static void enter_raw(void)
{
    if (!isatty(STDIN_FILENO))
        return;

    tcgetattr(STDIN_FILENO, &tui_saved);

    struct termios raw = tui_saved;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    tui_raw = 1;
}

static void leave_raw(void)
{
    if (tui_raw)
        tcsetattr(STDIN_FILENO, TCSANOW, &tui_saved);

    tui_raw = 0;
}

static int term_width(void)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;

    return 80;
}

static int read_key(void)
{
    unsigned char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return 'q';

    if (c == 3)
        return 'q';

    if (c == '\r' || c == '\n')
        return c;

    if (c == 'k')
        return KEY_UP;

    if (c == 'j')
        return KEY_DOWN;

    if (c == 27) {
        unsigned char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1 || read(STDIN_FILENO, &seq[1], 1) != 1)
            return 0;

        if (seq[0] == '[' && seq[1] == 'A')
            return KEY_UP;

        if (seq[0] == '[' && seq[1] == 'B')
            return KEY_DOWN;

        return 0;
    }

    return c;
}

static void hline(int width)
{
    fputs("\033[90m", stdout);

    for (int i = 0; i < width; i++)
        putchar('-');

    fputs("\033[0m\n", stdout);
}

static void draw(int selected, const char *status)
{
    int width = term_width();
    int text = width - 18;
    if (text < 1)
        text = 1;

    fputs("\033[2J\033[H", stdout);
    printf("\033[1;36mtinyctl\033[0m  \033[90mcontrol center for the tiny suite\033[0m\n");
    hline(width);

    for (int i = 0; i < component_count; i++) {
        if (i == selected)
            printf("\033[7m > %-12s %.*s\033[0m\n",
                   components[i].name, text, components[i].summary);
        else
            printf("   %-12s %.*s\n", components[i].name, text, components[i].summary);
    }

    hline(width);

    if (component_count > 0) {
        struct component *c = &components[selected];

        printf(" \033[1m%s\033[0m\n", c->name);
        printf("   summary: %s\n", c->summary);
        printf("   path:    %s\n", c->dir);

        if (c->num_deps > 0) {
            printf("   deps:    ");
            for (int i = 0; i < c->num_deps; i++)
                printf("%s%s", i ? " " : "", c->deps[i]);
            printf("\n");
        }

        printf("   build:   %s\n", c->build[0] ? c->build : "-");
        printf("   run:     %s\n", c->run[0] ? c->run : "-");
        printf("   test:    %s\n", c->test[0] ? c->test : "-");
        printf("   install: %s\n", c->install[0] ? c->install : "-");
    }

    hline(width);
    printf(" \033[1mb\033[0m build  \033[1mr\033[0m run  \033[1mt\033[0m test  "
           "\033[1mi\033[0m install  \033[1mu\033[0m uninstall  "
           "\033[1md\033[0m doctor  \033[1mq\033[0m quit\n");

    if (status && status[0])
        printf(" \033[33m%s\033[0m\n", status);

    fputs("\033[?25l", stdout);
    fflush(stdout);
}

static void wait_enter(void)
{
    unsigned char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n' && c != '\r')
        ;
}

static void run_action(struct component *c, const char *label, const char *command, char *status)
{
    if (!command || !command[0]) {
        snprintf(status, 128, "%s is not supported for %s", label, c->name);
        return;
    }

    leave_raw();
    fputs("\033[2J\033[H", stdout);
    printf("tinyctl: %s %s\n", label, c->name);
    fflush(stdout);

    run_in(c->dir, command);

    fputs("\n\033[90mpress enter to return\033[0m", stdout);
    fflush(stdout);

    wait_enter();
    enter_raw();
}

static void run_doctor(void)
{
    leave_raw();
    fputs("\033[2J\033[H", stdout);

    cmd_doctor();

    fputs("\n\033[90mpress enter to return\033[0m", stdout);
    fflush(stdout);

    wait_enter();
    enter_raw();
}

int tui_run(void)
{
    if (component_count == 0) {
        fprintf(stderr, "tinyctl: no components found in %s\n", suite_root);
        return 1;
    }

    enter_raw();
    fputs("\033[?1049h", stdout);

    int selected = 0;
    int running = 1;
    char status[128] = "";

    while (running) {
        draw(selected, status);
        status[0] = '\0';

        int key = read_key();

        switch (key) {
        case 'q':
            running = 0;
            break;
        case KEY_UP:
            if (selected > 0)
                selected--;
            break;
        case KEY_DOWN:
            if (selected + 1 < component_count)
                selected++;
            break;
        case 'b':
            run_action(&components[selected], "build", components[selected].build, status);
            break;
        case 'r':
            run_action(&components[selected], "run", components[selected].run, status);
            break;
        case 't':
            run_action(&components[selected], "test", components[selected].test, status);
            break;
        case 'i':
            run_action(&components[selected], "install", components[selected].install, status);
            break;
        case 'u':
            run_action(&components[selected], "uninstall", components[selected].uninstall, status);
            break;
        case 'd':
            run_doctor();
            break;
        default:
            break;
        }
    }

    fputs("\033[?1049l\033[?25h", stdout);
    fflush(stdout);
    leave_raw();

    return 0;
}