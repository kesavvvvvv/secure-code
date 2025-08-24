#ifndef SANDBOX_H
#define SANDBOX_H

#define SANDBOX_ROOT "sandbox_root"
#define JAIL_PROG    "/userprog"

#define LOG_OUT "logs/run_out.txt"
#define LOG_ERR "logs/run_err.txt"
#define LOG_COMP_ERR "logs/compile_err.txt"

int compile_user_code(void);
int run_in_sandbox(void);

#endif

