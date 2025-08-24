// main.c
// Minimal TUI: enter code, compile, run, show log hints.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sandbox.h"
#include "util.h"

static void menu(void) {
    puts("\n=== SecureCode Sandbox ===");
    puts("1) Enter/Edit C code");
    puts("2) Compile & Run");
    puts("3) Quit");
    printf("Select: ");
    fflush(stdout);
}

static int edit_code(void) {
    ensure_dirs();

    printf("\nEnter C program. End with a single line containing only <<<END>>>.\n");
    printf("(Tip: paste your code, then type <<<END>>> on its own line)\n\n");

    FILE *f = fopen("workspace/source.c", "w");
    if (!f) { perror("fopen workspace/source.c"); return -1; }

    char line[8192];
    while (1) {
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strcmp(line, "<<<END>>>\n") == 0 || strcmp(line, "<<<END>>>") == 0) break;
        fputs(line, f);
    }
    fclose(f);
    printf("Saved to workspace/source.c\n");
    return 0;
}

int main(void) {
    ensure_dirs();

    while (1) {
        menu();
        int opt = 0;
        if (scanf("%d%*c", &opt) != 1) break;

        if (opt == 1) {
            edit_code();
        } else if (opt == 2) {
            printf("Compiling...\n");
            int rc = compile_user_code();
            if (rc == 0) {
                printf("Compilation OK.\n");
                printf("Preparing sandbox & running...\n");
                if (run_in_sandbox() == 0) {
                    puts("Run done.");
                } else {
                    puts("Run failed. Check logs/run_err.txt");
                }
            } else if (rc == 1) {
                printf("Compilation failed. See %s\n", LOG_COMP_ERR);
            } else {
                printf("Compiler error (internal).\n");
            }
            printf("\nOpen logs:\n - %s (stdout)\n - %s (stderr)\n - %s (gcc)\n",
                   LOG_OUT, LOG_ERR, LOG_COMP_ERR);
        } else if (opt == 3) {
            break;
        } else {
            puts("Invalid option.");
        }
    }
    return 0;
}

