#include <conf.h>
#include <stdlib.h>
#include <ini/ini.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

void livd_conf_init(livd_conf_t *config) {
    if (config == NULL) return;

    *config = (livd_conf_t){
        .restart = LIVD_RESTART_ON_FAILURE,
        .restart_delay = 1,
        .max_restarts = 5
    };
}

void livd_conf_destroy(livd_conf_t *config) {
    if (config == NULL) return;

    free(config->name);
    free(config->command);

    for (size_t i = 0; i < config->arg_count; i++) {
        free(config->args[i]);
    }

    free(config->args);
    livd_conf_init(config);
}

static int livd_conf_set_string(char **dst, const char *value) {
    if (*dst != NULL || value == NULL || value[0] == '\0') return 0;

    *dst = strdup(value);
    return *dst != NULL;
}

static int livd_conf_add_arg(livd_conf_t *config, const char *value) {
    char *arg = strdup(value);
    if (arg == NULL) return 0;

    char **args = realloc(config->args, sizeof(*args) * (config->arg_count + 1));
    if (args == NULL) {
        free(arg);
        return 0;
    }

    config->args = args;
    config->args[config->arg_count++] = arg;
    return 1;
}

static int livd_conf_handler(void *user, const char *section, const char *name, const char *value) {
    livd_conf_t *config = user;

    if (strcmp(section, "service") != 0) return 0;

    if (strcmp(name, "name") == 0) return livd_conf_set_string(&config->name, value);
    if (strcmp(name, "command") == 0) return livd_conf_set_string(&config->command, value);
    if (strcmp(name, "arg") == 0) return value != NULL && livd_conf_add_arg(config, value);

    if (strcmp(name, "restart") == 0) {
        if (strcmp(value, "never") == 0) config->restart = LIVD_RESTART_NEVER;
        else if (strcmp(value, "always") == 0) config->restart = LIVD_RESTART_ALWAYS;
        else if (strcmp(value, "on-failure") == 0) config->restart = LIVD_RESTART_ON_FAILURE;
        else return 0;
        return 1;
    }

    if (strcmp(name, "restart-delay") == 0 || strcmp(name, "max-restarts") == 0) {
        char *end = NULL;
        errno = 0;
        unsigned long number = strtoul(value, &end, 10);
        if (errno != 0 || end == value || *end != '\0' || number > UINT_MAX) return 0;

        if (strcmp(name, "restart-delay") == 0) config->restart_delay = (unsigned int)number;
        else config->max_restarts = (unsigned int)number;
        return 1;
    }

    return 0;
}

int livd_conf_load(livd_conf_t *config, const char *path) {
    if (config == NULL || path == NULL) return -1;

    int result = ini_parse(path, livd_conf_handler, config);

    if (result != 0) {
        fprintf(stderr, "livd: failed to parse config '%s' (error %d)\n", path, result);
        livd_conf_destroy(config);
        return -1;
    }

    if (config->name == NULL || config->command == NULL) {
        fprintf(stderr, "livd: config must specify name and command\n");
        livd_conf_destroy(config);
        return -1;
    }

    return 0;
}
