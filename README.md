# SecureCode: A Code Execution Sandbox Using System Calls

## What it does
- Lets you paste C code in a simple terminal interface.
- Compiles it with `gcc` and captures compiler output to `logs/compile_*`.
- Runs the program **inside a chroot jail** with **CPU time & memory limits**.
- Captures program stdout/stderr to `logs/run_*`.
- Uses ONLY these system calls for sandboxing logic: `open, read, write, close, fork, execvp, waitpid, alarm, kill, dup2, setrlimit, chroot`.

## Build & Run
```bash
make
sudo make run

