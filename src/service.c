#include <service.h>
#include <conf.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void livd_serv_init(livd_serv_t* service) {
    if (!service) return;

    livd_conf_init(&service->config);
    service->pid = -1;
    service->restarts = 0;

    service->running = false;
    service->failed = false;
}

pid_t livd_serv_start(livd_serv_t* service) {
    if (!service) return -1;
    if (service->running) return -1;
    if (!service->config.command) return -1;

    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return 1;
    } else if (child == 0) {
        printf("I'm a child!\n");

        if (execvp(service->config.command, child_argv)) {
                int err = errno;
                perror("execvp");
        }

        _exit(127);
    } else {
        int status;
        bool failed = false;

        printf("I'm a parent!\n");

        if (waitpid(child, &status, 0) == -1) {
                perror("waitpid");
                return 1;
        }

        if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);

                if (code == 0) {
                        printf("Child exited with good exit: %d\n", code);
                } else {
                        printf("Child exited with bad exit: %d\n", code);
                        failed = true;
                }
        }

        if (WIFSIGNALED(status)) {
                printf("Child exited abnormally because of signal: %d\n", WTERMSIG(status));
                failed = true;
        }

        bool should_restart = false;

        switch (config.restart) {
                case LIVD_RESTART_NEVER:
                        should_restart = false;
                        break;
                case LIVD_RESTART_ALWAYS:
                        should_restart = true;
                        break;
                case LIVD_RESTART_ON_FAILURE:
                        should_restart = failed;
                        break;
        }

        printf("Child finished; parent continuing!\n");
        free(child_argv);
        livd_conf_destroy(&config);
    }
}
