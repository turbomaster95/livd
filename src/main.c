#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <conf.h>
#include <service.h>

int fd[2];

int main(int argc, char** argv) {
    printf("[livd] starting..\n");
    fflush(stdout);

    if (argc != 3 || strcmp(argv[1], "-c") != 0) {
        fprintf(stderr, "Usage: %s -c <config-file>\n", argv[0]);
        return -1;
    }

    livd_conf_t config;
    livd_conf_init(&config);

    if (livd_conf_load(&config, argv[2]) == -1) {
        livd_conf_destroy(&config);
        return -1;
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return -1;
    }

    for (int i = 0; i < 2; i++) {
         printf("Opened pipe fd[%d]: %d\n", i, fd[i]);
    }

    int flags = fcntl(fd[1], F_GETFD);
    if (flags == -1 || fcntl(fd[1], F_SETFD, flags | FD_CLOEXEC) == -1) {
         perror("fcntl");
         return 1;
    }

    char **child_argv = calloc(config.arg_count + 2, sizeof(*child_argv));
    if (child_argv == NULL) {
        perror("calloc");
        livd_conf_destroy(&config);
        return -1;
    }

    child_argv[0] = config.command;
    for (size_t i = 0; i < config.arg_count; i++) child_argv[i + 1] = config.args[i];

    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return 1;
    } else if (child == 0) {
        printf("I'm a child!\n");
        close(fd[0]);

        if (execvp(config.command, child_argv)) {
                int err = errno;
                perror("execvp");
                write(fd[1], &err, sizeof(err));
        }

        _exit(127);
    } else {
        close(fd[1]);
        int status;
	bool failed = false;

        printf("I'm a parent!\n");

        int rerr;

        ssize_t n = read(fd[0], &rerr, sizeof(rerr));
        if (n == -1) {
                perror("read");
                return -1;
        } else if (n == sizeof(rerr)) {
                printf("Error: %s\n", strerror(rerr));
		failed = true;
        } else if (n > 0 && n != sizeof(rerr)) {
                printf("Incomplete error code received from pipe!");
        }

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

    return 0;
}
