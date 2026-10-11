#ifndef TINYCTL_H
#define TINYCTL_H

#include <stddef.h>

#define MAX_COMPONENTS 64
#define MAX_DEPS 16

struct component {
    char dir[1024];
    char name[64];
    char summary[256];
    char build[256];
    char run[256];
    char test[256];
    char install[256];
    char uninstall[256];
    char deps[MAX_DEPS][64];
    int num_deps;
};

extern struct component components[MAX_COMPONENTS];
extern int component_count;
extern char suite_root[512];

void scan_components(void);
struct component *find_component(const char *name);

int run_in(const char *dir, const char *command);
int have(const char *command);
int have_pkg(const char *package);

int cmd_list(void);
int cmd_info(const char *name);
int cmd_doctor(void);
int cmd_action(const char *action, const char *name);

int tui_run(void);

#endif