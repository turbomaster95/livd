#ifndef LIVD_CONF_H
#define LIVD_CONF_H

#include <stddef.h>

typedef enum {
    LIVD_RESTART_NEVER,
    LIVD_RESTART_ALWAYS,
    LIVD_RESTART_ON_FAILURE
} livd_restart_policy_t;

typedef struct {
    char *name;
    char *command;
    char **args;
    size_t arg_count;
    livd_restart_policy_t restart;
    unsigned int restart_delay;
    unsigned int max_restarts;
} livd_conf_t;

void livd_conf_init(livd_conf_t *config);
int  livd_conf_load(livd_conf_t *config, const char *path);
void livd_conf_destroy(livd_conf_t *config);

#endif
