// sandbox.c
// Sets limits, chroots, redirects IO, executes /userprog inside jail.

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "sandbox.h"
#include "util.h"

static void set_limits(void) {
    struct rlimit rl;

    // CPU time limit: 2 seconds
    rl.rlim_cur = 2;
    rl.rlim_max = 2;
    setrlimit(RLIMIT_CPU, &rl);

    // Address space limit: 256 MB
    rl.rlim_cur = 256 * 1024 * 1024;
    rl.rlim_max = 256 * 1024 * 1024;
    setrlimit(RLIMIT_AS, &rl);

    // File size limit: 8 MB
    rl.rlim_cur = 8 * 1024 * 1024;
    rl.rlim_max = 8 * 1024 * 1024;
    setrlimit(RLIMIT_FSIZE, &rl);

    // Disable core dumps
    rl.rlim_cur = 0;
    rl.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rl);

    alarm(3); // hard wall
}

int run_in_sandbox(void) {
    ensure_dirs();

    // Clear run logs every time
    int fd_out = open(LOG_OUT, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (fd_out >= 0) close(fd_out);
    int fd_err = open(LOG_ERR, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (fd_err >= 0) close(fd_err);

    // Always refresh the jail binary
    if (prepare_jail_files("build/userprog", JAIL_PROG, SANDBOX_ROOT) != 0) {
        // Already logged by util.c
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        // CHILD: wire stdout/stderr to logs (opened outside jail)
        int out = open(LOG_OUT, O_WRONLY | O_APPEND);
        int err = open(LOG_ERR, O_WRONLY | O_APPEND);
        if (out >= 0) { dup2(out, STDOUT_FILENO); close(out); }
        if (err >= 0) { dup2(err, STDERR_FILENO); close(err); }

        setbuf(stdout, NULL);
        setbuf(stderr, NULL);

        set_limits();

        // chroot jail
        if (chdir(SANDBOX_ROOT) != 0) {
            perror("[WARN] chdir SANDBOX_ROOT");
        }
        if (chroot(".") != 0) {
            perror("[WARN] chroot (run with sudo for full isolation)");
        }
        if (chdir("/") != 0) {
            perror("[WARN] chdir /");
        }

        // Execute the user program inside jail
        char *const argv[] = { (char*)"/userprog", NULL };
        execv("/userprog", argv);

        // If exec fails, report and exit
        perror("execv /userprog");
        _exit(127);
    }

    // PARENT: wait and log result into run_err
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) {
        int efd = open(LOG_ERR, O_WRONLY | O_APPEND);
        if (efd >= 0) {
            dprintf(efd, "[ERR] waitpid failed: %s\n", strerror(errno));
            close(efd);
        }
        return -1;
    }

    int efd = open(LOG_ERR, O_WRONLY | O_APPEND);
    if (efd >= 0) {
        if (WIFEXITED(st)) {
            dprintf(efd, "[INFO] Child exited with code %d\n", WEXITSTATUS(st));
        } else if (WIFSIGNALED(st)) {
            dprintf(efd, "[INFO] Child terminated by signal %d\n", WTERMSIG(st));
        } else {
            dprintf(efd, "[INFO] Child ended (unknown state)\n");
        }
        close(efd);
    }
    return 0;
}

