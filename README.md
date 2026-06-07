# SecureCode: A Code Execution Sandbox Using System Calls

A lightweight C-based sandbox that safely executes untrusted C code with resource limits and filesystem isolation using Linux system calls.

## Overview

**SecureCode** is an interactive command-line tool that allows you to:

1. **Enter/Edit C code** through a simple terminal interface
2. **Compile code** safely using `gcc` with compiler output captured to logs
3. **Execute code** inside an isolated **chroot jail** with strict resource limits
4. **Monitor execution** via captured stdout/stderr logs

This project demonstrates secure code execution principles using **only low-level system calls** for sandboxing logic: `open`, `read`, `write`, `close`, `fork`, `execvp`, `waitpid`, `alarm`, `kill`, `dup2`, `setrlimit`, and `chroot`.

---

## Features

### Security & Isolation
- **chroot jail**: Execution runs in an isolated filesystem root (`sandbox_root/`)
- **CPU time limit**: Programs are terminated after 2 seconds of CPU time
- **Memory limit**: Maximum 256 MB address space per process
- **File size limit**: Programs cannot write more than 8 MB
- **Core dump disabled**: Prevents sensitive data leakage
- **Wall clock timeout**: Hard 3-second wall-clock limit via `alarm()`

### User Experience
- **Interactive TUI**: Menu-driven interface for code entry, compilation, and execution
- **Persistent logging**: All output captured to timestamped log files
  - `logs/run_out.txt` - Program stdout
  - `logs/run_err.txt` - Program stderr & execution status
  - `logs/compile_err.txt` - Compiler errors and warnings
- **Code persistence**: Source code saved to `workspace/source.c` for easy re-editing

### Compilation
- Attempts static linking first (for portability)
- Falls back to dynamic linking if static fails
- Captures all compiler warnings and errors
- Builds to `build/userprog` for sandbox execution

---

## Project Structure

```
secure-code/
├── include/
│   ├── sandbox.h          # Sandbox constants & declarations
│   └── util.h             # Utility function declarations
├── src/
│   ├── main.c             # Interactive menu & TUI
│   ├── sandbox.c          # Resource limits & chroot execution
│   ├── compile.c          # GCC compilation wrapper
│   └── util.c             # File utilities & path validation
├── build/                 # Compiled binary output (generated)
├── logs/                  # Execution logs (generated)
│   ├── run_out.txt
│   ├── run_err.txt
│   └── compile_err.txt
├── workspace/             # User code storage (generated)
│   └── source.c
├── sandbox_root/          # Chroot jail directory (generated)
├── Makefile               # Build configuration
└── README.md              # This file
```

---

## Build & Run

### Prerequisites
- Linux system (chroot and `setrlimit` required)
- `gcc` compiler
- `make` utility
- `sudo` (for full chroot isolation)

### Compilation

```bash
# Compile the SecureCode executable
make
```

This will:
- Create required directories (`build/`, `logs/`, `workspace/`, `sandbox_root/`)
- Compile all source files with optimizations and strict warnings
- Generate `build/securecode` binary

### Execution

```bash
# Run with sudo for full filesystem isolation
sudo ./build/securecode
```

The program will present an interactive menu:

```
=== SecureCode Sandbox ===
1) Enter/Edit C code
2) Compile & Run
3) Quit
Select: 
```

### Cleanup

```bash
# Remove build artifacts and logs
make clean
```

---

## Usage Guide

### Step 1: Enter C Code

Select option `1` from the menu. The program will prompt:

```
Enter C program. End with a single line containing only <<<END>>>.
(Tip: paste your code, then type <<<END>>> on its own line)
```

Paste your C code and terminate with `<<<END>>>` on its own line.

**Example:**
```c
#include <stdio.h>

int main() {
    printf("Hello, Secure World!\n");
    return 0;
}
<<<END>>>
```

Your code is saved to `workspace/source.c`.

### Step 2: Compile & Run

Select option `2` from the menu. The program will:

1. **Compile** your code using `gcc`
2. **Check for errors** (if compilation fails, examine `logs/compile_err.txt`)
3. **Set up the sandbox** and copy the binary into the chroot jail
4. **Execute** your program with resource limits
5. **Display log locations** for output inspection

### Step 3: Review Logs

After execution, check the generated logs:

```bash
# Program output (stdout)
cat logs/run_out.txt

# Program errors & execution status (stderr)
cat logs/run_err.txt

# Compiler warnings and errors
cat logs/compile_err.txt
```

### Step 4: Iterate

Go back to step 1 to edit and re-run your code.

---

## System Calls Used

SecureCode uses only the following system calls for its sandboxing implementation:

| System Call | Purpose |
|------------|---------|
| `open` | Open/create files for logging |
| `read` | Read code from user input |
| `write` | Write logs and program output |
| `close` | Close file descriptors |
| `fork` | Create child processes for compiler and user program |
| `execvp` | Execute gcc compiler and user program |
| `waitpid` | Wait for child process completion |
| `alarm` | Set wall-clock timeout |
| `kill` | Terminate processes exceeding time limits |
| `dup2` | Redirect stdout/stderr to log files |
| `setrlimit` | Set CPU time, memory, and file size limits |
| `chroot` | Isolate process to sandbox root directory |

---

## Key Implementation Details

### Resource Limits (src/sandbox.c)

```c
CPU time limit:     2 seconds    (RLIMIT_CPU)
Memory limit:       256 MB       (RLIMIT_AS)
File size limit:    8 MB         (RLIMIT_FSIZE)
Core dump limit:    0 bytes      (RLIMIT_CORE)
Wall-clock timeout: 3 seconds    (alarm())
```

### Path Validation (src/util.c)

The `path_is_within_root()` function prevents directory traversal attacks by:
- Resolving both real and symbolic paths
- Verifying that all file operations stay within `sandbox_root/`
- Handling non-existent paths by validating their parent directory

### Compilation (src/compile.c)

1. Attempts static linking for portability
2. Falls back to dynamic linking if needed
3. Captures compiler stderr to `logs/compile_err.txt`
4. Returns 0 on success, 1 on compiler error

### Sandbox Execution (src/sandbox.c)

1. Clears previous logs
2. Copies compiled binary into chroot jail
3. Forks a child process
4. Child process:
   - Opens and redirects stdout/stderr to log files
   - Sets resource limits via `setrlimit()`
   - Changes root directory via `chroot()`
   - Executes user program
5. Parent process waits and logs termination status

---

## Example: CPU Limit Test

When a program exceeds the 2-second CPU limit, it is terminated by signal 9 (SIGKILL):

```
=== CPU LIMIT TEST START ===
Still running... counter=100000000
Still running... counter=200000000
...
Still running... counter=4700000000
[INFO] Child terminated by signal 9
```

Output: `logs/run_err.txt`

---

## Security Considerations

### What This Sandbox Protects Against
- **Infinite loops** - CPU time limit terminates runaway code
- **Memory exhaustion** - Address space limit prevents allocation DoS
- **Filesystem escape** - chroot jail + path validation prevent directory traversal
- **Denial of service** - Wall-clock alarm ensures bounded execution time
- **Information leakage** - Core dumps disabled to prevent binary exposure

### Limitations
- **Requires sudo** - Full isolation requires root privileges
- **Single-user design** - Not intended for multi-tenant scenarios
- **No network isolation** - Programs can access network (disable if needed)
- **No PID/IPC isolation** - Use namespaces for stronger isolation
- **Compile-time security** - Compiler flags (`-Wall -Wextra`) provide warnings only

---

## Compilation Flags

```makefile
-O2             # Optimization level 2
-Wall -Wextra   # Enable all common warnings
-std=c11        # Use C11 standard
-Iinclude       # Include directory
-static         # Static linking (attempted first)
```

---

## Log File Locations

| File | Purpose |
|------|---------|
| `logs/run_out.txt` | Program standard output |
| `logs/run_err.txt` | Program standard error + execution status |
| `logs/compile_err.txt` | GCC compiler warnings and errors |

---

## Troubleshooting

### "Permission denied" on chroot
- Ensure you run with `sudo`
- Run: `sudo ./build/securecode`

### Compilation fails silently
- Check `logs/compile_err.txt` for compiler errors
- Verify syntax of your C code
- Ensure `gcc` is installed: `gcc --version`

### Program times out
- Your code exceeded the 2-second CPU limit
- Check `logs/run_err.txt` for signal information
- Optimize your algorithm or adjust limits in `src/sandbox.c`

### Memory limit exceeded
- Your program tried to allocate more than 256 MB
- Reduce memory usage or increase `RLIMIT_AS` in `src/sandbox.c`

### File size too large
- Output file exceeded 8 MB limit
- Check what's being written to disk in your program

---

## Future Enhancements

- [ ] Network isolation via network namespaces
- [ ] Process tracing for profiling (`ptrace`)
- [ ] Support for other languages (Python, JavaScript)
- [ ] Web interface for remote code execution
- [ ] Configurable resource limits per execution
- [ ] Process death reason analysis (exit code, signal handling)
- [ ] Code syntax highlighting in TUI

---

## License

MIT

---

## Author

Created by **kesavvvvvv**

Repository: [kesavvvvvv/secure-code](https://github.com/kesavvvvvv/secure-code)

---

## References

- Linux `chroot(2)` documentation
- POSIX resource limits: `setrlimit(2)`
- C standard library: `fork(2)`, `execve(2)`, `wait(2)`
- GCC compiler documentation
