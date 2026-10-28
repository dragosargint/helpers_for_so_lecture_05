#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

// 1 Gigabyte in bytes (1024 * 1024 * 1024)
#define ONE_GB (1ULL << 30)
#define ONE_MB (1ULL << 20)
#define SIZE ONE_GB
// Signal handler to wake up pause() in child processes
void handle_sigusr1(int sig) {
    (void)sig;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Error: Number of forks must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    // Get the system page size (typically 4096 bytes / 4KB)
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    pid_t *pids = malloc(n * sizeof(pid_t));
    if (!pids) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // 1. Allocate 1GB of shared anonymous memory
    char *shared_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, 
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        free(pids);
        exit(EXIT_FAILURE);
    }

    printf("Parent (PID %d) allocated 1GB shared memory.\n", getpid());

    // 2. Make memory resident: touch every page (1 byte per 4KB page = ~262,144 pages)
    printf("Parent populating/faulting in all pages to force physical RAM allocation...\n");
    for (size_t offset = 0; offset < SIZE; offset += page_size) {
        shared_mem[offset] = 'P'; // Initialized by Parent
    }
    printf("1GB memory is now resident in RAM.\n\n");

    // 3. Fork n times
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // --- CHILD PROCESS ---
            struct sigaction sa = {0};
            sa.sa_handler = handle_sigusr1;
            sigaction(SIGUSR1, &sa, NULL);

            char fill_char = 'A' + (i % 26);
            printf("Child %d (PID %d) writing '%c' across all 1GB pages...\n", 
                   i + 1, getpid(), fill_char);
            fflush(stdout);

            // Write to every single page in the 1GB region
            for (size_t offset = 0; offset < SIZE; offset += page_size) {
                // Each child offsets slightly within the page to avoid clobbering exact index
                shared_mem[offset + (i % page_size)] = fill_char;
            }

            printf("Child %d (PID %d) done writing to all pages. Waiting for finish signal...\n", 
                   i + 1, getpid());
            fflush(stdout);

            // Wait until parent sends SIGUSR1
            pause();

            printf("Child %d (PID %d) exiting.\n", i + 1, getpid());
            fflush(stdout);

            free(pids);
            exit(EXIT_SUCCESS);
        } else {
            // --- PARENT PROCESS ---
            pids[i] = pid;
        }
    }

    // --- PARENT PROCESS CONTINUED ---

    // 4. Wait for user input
    printf("\nAll %d children spawned and actively holding the shared memory pages.\n", n);
    printf("Press [Enter] to send finish command to all children...");
    getchar();

    // 5. Signal all children to finish
    printf("\nSending finish command (SIGUSR1) to children...\n");
    for (int i = 0; i < n; i++) {
        kill(pids[i], SIGUSR1);
    }

    // 6. Wait for children to exit
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    // 7. Verify modifications from child processes on page 0 and page 1
    printf("\nAll children finished. Parent inspecting sample shared memory pages:\n");
    for (int i = 0; i < n; i++) {
        printf("Page 0, offset %d byte: '%c'\n", i, shared_mem[i]);
    }

    // 8. Cleanup
    if (munmap(shared_mem, SIZE) == -1) {
        perror("munmap failed");
    }
    free(pids);

    printf("Parent unmapped memory and exiting.\n");
    return 0;
}
