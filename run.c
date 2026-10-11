#include "tinyctl.h"
#include <stdio.h>
#include <stdlib.h>

int run_in(const char *dir, const char *command)
{
    char full[2048];
    snprintf(full, sizeof(full), "cd '%s' && %s", dir, command);
    return system(full);
}

int have(const char *command)
{
    char full[256];
    snprintf(full, sizeof(full), "command -v %s >/dev/null 2>&1", command);
    return system(full) == 0;
}

int have_pkg(const char *package)
{
    char full[256];
    snprintf(full, sizeof(full), "pkg-config --exists %s 2>/dev/null", package);
    return system(full) == 0;
}