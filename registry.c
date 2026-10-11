#include "tinyctl.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct component components[MAX_COMPONENTS];
int component_count;
char suite_root[512];

static void trim(char *s)
{
    int len = (int)strlen(s);

    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                       s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }

    int start = 0;
    while (s[start] == ' ' || s[start] == '\t')
        start++;

    if (start > 0)
        memmove(s, s + start, strlen(s + start) + 1);
}

static void strip_quotes(char *s)
{
    int len = (int)strlen(s);

    if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

static void copy_field(char *dst, size_t size, const char *src)
{
    size_t len = strlen(src);

    if (len >= size)
        len = size - 1;

    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void split_deps(struct component *c, char *value)
{
    char *token = strtok(value, " \t,");

    while (token && c->num_deps < MAX_DEPS) {
        copy_field(c->deps[c->num_deps], sizeof(c->deps[0]), token);
        c->num_deps++;
        token = strtok(NULL, " \t,");
    }
}

static void parse_component(const char *path, struct component *c)
{
    FILE *file = fopen(path, "r");
    if (!file)
        return;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#')
            continue;

        char *eq = strchr(line, '=');
        if (!eq)
            continue;

        *eq = '\0';

        char *key = line;
        char *value = eq + 1;

        trim(key);
        trim(value);
        strip_quotes(value);

        if (strcmp(key, "name") == 0)
            copy_field(c->name, sizeof(c->name), value);
        else if (strcmp(key, "summary") == 0)
            copy_field(c->summary, sizeof(c->summary), value);
        else if (strcmp(key, "build") == 0)
            copy_field(c->build, sizeof(c->build), value);
        else if (strcmp(key, "run") == 0)
            copy_field(c->run, sizeof(c->run), value);
        else if (strcmp(key, "test") == 0)
            copy_field(c->test, sizeof(c->test), value);
        else if (strcmp(key, "install") == 0)
            copy_field(c->install, sizeof(c->install), value);
        else if (strcmp(key, "uninstall") == 0)
            copy_field(c->uninstall, sizeof(c->uninstall), value);
        else if (strcmp(key, "deps") == 0)
            split_deps(c, value);
    }

    fclose(file);
}

void scan_components(void)
{
    DIR *dir = opendir(suite_root);
    if (!dir)
        return;

    struct dirent *entry;

    while ((entry = readdir(dir)) && component_count < MAX_COMPONENTS) {
        if (entry->d_name[0] == '.')
            continue;

        char path[2048];
        snprintf(path, sizeof(path), "%s/%s/component", suite_root, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0)
            continue;

        struct component *c = &components[component_count];
        memset(c, 0, sizeof(*c));
        snprintf(c->dir, sizeof(c->dir), "%s/%s", suite_root, entry->d_name);

        parse_component(path, c);

        if (c->name[0] == '\0')
            copy_field(c->name, sizeof(c->name), entry->d_name);

        component_count++;
    }

    closedir(dir);
}

struct component *find_component(const char *name)
{
    for (int i = 0; i < component_count; i++) {
        if (strcmp(components[i].name, name) == 0)
            return &components[i];
    }

    return NULL;
}