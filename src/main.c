#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int fd[2];

int main(int argc, char** argv) {
    printf("[livd] starting..\n");
    fflush(stdout);

    if (argc < 2) return -1;

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

    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return 1;
    } else if (child == 0) {
        printf("I'm a child!\n");
        close(fd[0]);

        if (execvp(argv[1], &argv[1])) {
                int err = errno;
                perror("execvp");
                write(fd[1], &err, sizeof(err));
        }

        _exit(127);
    } else {
        close(fd[1]);
        int status;

        printf("I'm a parent!\n");

        int rerr;

        ssize_t n = read(fd[0], &rerr, sizeof(rerr));
        if (n == -1) {
                perror("read");
                return -1;
        } else if (n == sizeof(rerr)) {
                printf("Error: %s\n", strerror(rerr));
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
                }
        }

        if (WIFSIGNALED(status)) {
                printf("Child exited abnormally because of signal: %d\n", WTERMSIG(status));
        }


        printf("Child finished; parent continuing!\n");
    }

    return 0;
}
