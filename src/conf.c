#include <conf.h>
#include <stdlib.h>
#include <ini/ini.h>

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
