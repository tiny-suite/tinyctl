#include "tinyctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TINYSUITE_ROOT
#define TINYSUITE_ROOT "."
#endif

#define VERSION "0.1"

static void usage(void)
{
    printf("tinyctl %s - control center for the tiny suite\n\n", VERSION);
    printf("usage: tinyctl [command] [component] [args]\n\n");
    printf("run with no arguments to open the interactive tui.\n\n");
    printf("commands:\n");
    printf("  tui                  open the interactive tui\n");
    printf("  list                 list components\n");
    printf("  info <component>     show component details\n");
    printf("  doctor               check build dependencies\n");
    printf("  build <component>    build a component\n");
    printf("  run <component>      run a component\n");
    printf("  test <component>     smoke test a component\n");
    printf("  install <component>  install a component\n");
    printf("  uninstall <component> remove a component\n");
    printf("  where <component>    print component path\n\n");
    printf("environment:\n");
    printf("  TINYSUITE_ROOT       path to the tiny suite (default %s)\n", TINYSUITE_ROOT);
}

static int cmd_where(const char *name)
{
    struct component *c = find_component(name);
    if (!c) {
        fprintf(stderr, "tinyctl: unknown component: %s\n", name);
        return 1;
    }

    printf("%s\n", c->dir);
    return 0;
}

int main(int argc, char **argv)
{
    const char *root = getenv("TINYSUITE_ROOT");
    if (!root)
        root = TINYSUITE_ROOT;
    snprintf(suite_root, sizeof(suite_root), "%s", root);

    scan_components();

    if (argc < 2) {
        if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
            return tui_run();

        return cmd_list();
    }

    const char *command = argv[1];

    if (strcmp(command, "tui") == 0)
        return tui_run();
    if (strcmp(command, "list") == 0)
        return cmd_list();
    if (strcmp(command, "doctor") == 0)
        return cmd_doctor();

    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0 ||
        strcmp(command, "--help") == 0) {
        usage();
        return 0;
    }

    if (strcmp(command, "version") == 0 || strcmp(command, "--version") == 0) {
        printf("tinyctl %s\n", VERSION);
        return 0;
    }

    if (argc < 3) {
        fprintf(stderr, "tinyctl: %s needs a component name\n", command);
        return 1;
    }

    const char *name = argv[2];

    if (strcmp(command, "info") == 0)
        return cmd_info(name);
    if (strcmp(command, "where") == 0)
        return cmd_where(name);

    if (strcmp(command, "build") == 0 || strcmp(command, "run") == 0 ||
        strcmp(command, "test") == 0 || strcmp(command, "install") == 0 ||
        strcmp(command, "uninstall") == 0) {
        return cmd_action(command, name);
    }

    fprintf(stderr, "tinyctl: unknown command: %s\n", command);
    usage();
    return 1;
}