#include "tinyctl.h"
#include <stdio.h>
#include <string.h>

int cmd_list(void)
{
    printf("%-14s %s\n", "component", "summary");

    for (int i = 0; i < component_count; i++)
        printf("%-14s %s\n", components[i].name, components[i].summary);

    return 0;
}

int cmd_info(const char *name)
{
    struct component *c = find_component(name);
    if (!c) {
        fprintf(stderr, "tinyctl: unknown component: %s\n", name);
        return 1;
    }

    printf("name:    %s\n", c->name);
    printf("summary: %s\n", c->summary);
    printf("path:    %s\n", c->dir);
    printf("build:   %s\n", c->build[0] ? c->build : "-");
    printf("run:     %s\n", c->run[0] ? c->run : "-");
    printf("test:    %s\n", c->test[0] ? c->test : "-");
    printf("install: %s\n", c->install[0] ? c->install : "-");

    if (c->num_deps > 0) {
        printf("deps:    ");
        for (int i = 0; i < c->num_deps; i++)
            printf("%s%s", i ? " " : "", c->deps[i]);
        printf("\n");
    }

    return 0;
}

int cmd_doctor(void)
{
    int missing = 0;

    printf("tools:\n");

    const char *tools[] = { "gcc", "make", "pkg-config", "git", "cc" };

    for (size_t i = 0; i < sizeof(tools) / sizeof(tools[0]); i++) {
        int ok = have(tools[i]);
        printf("  %-12s %s\n", tools[i], ok ? "ok" : "missing");

        if (!ok && strcmp(tools[i], "git") != 0)
            missing++;
    }

    int any_deps = 0;
    for (int i = 0; i < component_count; i++)
        any_deps += components[i].num_deps;

    if (any_deps > 0) {
        printf("libraries:\n");

        for (int i = 0; i < component_count; i++) {
            for (int j = 0; j < components[i].num_deps; j++) {
                const char *dep = components[i].deps[j];
                int ok = have_pkg(dep);
                printf("  %-16s %s\n", dep, ok ? "ok" : "missing");

                if (!ok)
                    missing++;
            }
        }
    }

    if (missing == 0) {
        printf("\nall good, ready to build\n");
        return 0;
    }

    printf("\n%d missing - some components will not build\n", missing);
    return 1;
}

int cmd_action(const char *action, const char *name)
{
    struct component *c = find_component(name);
    if (!c) {
        fprintf(stderr, "tinyctl: unknown component: %s\n", name);
        return 1;
    }

    const char *command = NULL;

    if (strcmp(action, "build") == 0)
        command = c->build;
    else if (strcmp(action, "run") == 0)
        command = c->run;
    else if (strcmp(action, "test") == 0)
        command = c->test;
    else if (strcmp(action, "install") == 0)
        command = c->install;
    else if (strcmp(action, "uninstall") == 0)
        command = c->uninstall;

    if (!command || !command[0]) {
        fprintf(stderr, "tinyctl: %s does not support %s\n", name, action);
        return 1;
    }

    printf("tinyctl: %s %s\n", action, name);
    return run_in(c->dir, command) == 0 ? 0 : 1;
}