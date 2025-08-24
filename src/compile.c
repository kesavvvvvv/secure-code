// compile.c
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "sandbox.h"
#include "util.h"

int compile_user_code(void) {
    ensure_dirs();

    // Clear compile error log
    int cerr = open(LOG_COMP_ERR, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (cerr >= 0) close(cerr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        int fd_err = open(LOG_COMP_ERR, O_CREAT|O_TRUNC|O_WRONLY, 0644);
        if (fd_err >= 0) {
            dup2(fd_err, STDERR_FILENO);
            close(fd_err);
        }

        // Try static first, fallback to normal
        char *argv1[] = {
            "gcc",
            "-O2","-Wall","-Wextra","-std=c11",
            "-static",
            "-o","build/userprog",
            "workspace/source.c",
            NULL
        };
        execvp(argv1[0], argv1);

        // If static fails, try without -static
        char *argv2[] = {
            "gcc",
            "-O2","-Wall","-Wextra","-std=c11",
            "-o","build/userprog",
            "workspace/source.c",
            NULL
        };
        execvp(argv2[0], argv2);

        perror("execvp gcc");
        _exit(127);
    }

    int st=0;
    if (waitpid(pid, &st, 0) < 0) {
        perror("waitpid");
        return -1;
    }
    if (WIFEXITED(st) && WEXITSTATUS(st) == 0) return 0;
    return 1;
}

