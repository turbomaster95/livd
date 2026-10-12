#ifndef LIVD_SERVICE_H
#define LIVD_SERVICE_H

#include <stddef.h>
#include <conf.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
	livd_conf_t config;
	pid_t pid;
	unsigned int restarts;
	bool running;
	bool failed;
} livd_serv_t;

void livd_serv_init(livd_serv_t* service);
pid_t livd_serv_start(livd_serv_t* service);
void livd_serv_destroy(livd_serv_t* service);

#endif
