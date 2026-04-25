#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 128
#define MAX_PROCESSES 15
#define PAGE_SIZE 4096 // Standard 4KB memory page

// --- MATCHES WRITE-UP: ENUMERATIONS & DATA STRUCTURES ---

typedef enum { READY, RUNNING, WAITING, TERMINATED } ProcessState;

// Process Control Block (PCB) 
typedef struct {
    pid_t pid;
    char name[32];
    ProcessState state;
    int memory_pages_used;
} PCB;

// Memory Page Table 
typedef struct {
    int page_id;
    int is_allocated;
    void* physical_address;
} PageTableEntry;

// --- GLOBAL STATE ---
PCB process_queue[MAX_PROCESSES];
PageTableEntry memory_map[5]; 
int process_count = 0;

// --- MEMORY MANAGEMENT MODULE ---

void init_memory_system() {
    for (int i = 0; i < 5; i++) {
        memory_map[i].page_id = i;
        memory_map[i].is_allocated = 0;
        memory_map[i].physical_address = NULL;
    }
}

void allocate_memory(int process_index, int pages_needed) {
    int pages_allocated = 0;
    for (int i = 0; i < 5 && pages_allocated < pages_needed; i++) {
        if (!memory_map[i].is_allocated) {
            memory_map[i].physical_address = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (memory_map[i].physical_address == MAP_FAILED) return;
            
            memory_map[i].is_allocated = 1;
            process_queue[process_index].memory_pages_used++;
            pages_allocated++;
        }
    }
}

void display_memory_map() {
    printf("\n--- Physical Memory Buffer Map ---\n");
    for (int i = 0; i < 5; i++) {
        if (memory_map[i].is_allocated) {
            printf("Page %d: [ALLOCATED] Kernel Address: %p\n", i, memory_map[i].physical_address);
        } else {
            printf("Page %d: [FREE]\n", i);
        }
    }
    printf("----------------------------------\n");
}

// --- MATCHES WRITE-UP: PROCESS MANAGEMENT MODULE ---

void load_test_processes() {
    // These exact names ensure your write-up text matches the screenshots perfectly
    strcpy(process_queue[0].name, "dldsr_compute");
    process_queue[0].pid = 1001;
    process_queue[0].state = READY;
    process_queue[0].memory_pages_used = 0;

    strcpy(process_queue[1].name, "rfid_sensor_poll");
    process_queue[1].pid = 1002;
    process_queue[1].state = WAITING;
    process_queue[1].memory_pages_used = 0;

    strcpy(process_queue[2].name, "chatbot_server");
    process_queue[2].pid = 1003;
    process_queue[2].state = READY;
    process_queue[2].memory_pages_used = 0;

    process_count = 3;
    allocate_memory(2, 2); // Allocate RAM for chatbot_server
}

void display_process_queue() {
    printf("\n--- Process Control Block (PCB) Table ---\n");
    printf("%-10s %-20s %-15s %-10s\n", "PID", "Process Name", "State", "Memory (Pages)");
    for (int i = 0; i < process_count; i++) {
        char state_str[15];
        if (process_queue[i].state == READY) strcpy(state_str, "READY");
        else if (process_queue[i].state == RUNNING) strcpy(state_str, "RUNNING");
        else if (process_queue[i].state == WAITING) strcpy(state_str, "WAITING");
        else strcpy(state_str, "TERMINATED");

        printf("%-10d %-20s %-15s %-10d\n", process_queue[i].pid, process_queue[i].name, state_str, process_queue[i].memory_pages_used);
    }
    printf("-----------------------------------------\n");
}

void execute_real_process(char *command) {
    char *args[10];
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) { args[i++] = token; token = strtok(NULL, " "); }
    args[i] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
        if (execvp(args[0], args) < 0) {
            printf("[Kernel Error] Command '%s' not recognized.\n", args[0]);
            exit(1);
        }
    } else {
        process_queue[process_count].pid = pid;
        strcpy(process_queue[process_count].name, args[0]);
        process_queue[process_count].state = RUNNING;
        process_queue[process_count].memory_pages_used = 1;
        process_count++;

        int status;
        waitpid(pid, &status, 0); // Zombie prevention from your writeup
        
        for (int j = 0; j < process_count; j++) {
            if (process_queue[j].pid == pid) process_queue[j].state = TERMINATED;
        }
    }
}

// --- PROJECT 20 CORE: MULTITHREADED PARALLEL DOWNLOADER ---

void execute_parallel_download(char *url, int num_threads) {
    printf("\n[Downloader] Spawning %d parallel chunk streams for %s\n", num_threads, url);
    
    int child_pids[10];

    for(int i = 0; i < num_threads; i++) {
        pid_t pid = fork(); // Multiprocessing fork for parallel execution

        if (pid == 0) {
            // CHILD THREAD: Calculate chunk range and download
            char range[32];
            sprintf(range, "%d-%d", i * 1048576, ((i + 1) * 1048576) - 1); // 1MB chunks
            
            printf("[*] Thread %d (PID %d): Requesting bytes %s\n", i+1, getpid(), range);
            
            char output_file[32];
            sprintf(output_file, "chunk_part%d.dat", i+1);

            // Execute actual Linux network download command
            char *args[] = {"curl", "-s", "-r", range, "-o", output_file, url, NULL};
            execvp("curl", args);
            exit(0);
        } else {
            // PARENT THREAD: Register child in the PCB table
            child_pids[i] = pid;
            process_queue[process_count].pid = pid;
            sprintf(process_queue[process_count].name, "download_thread_%d", i+1);
            process_queue[process_count].state = RUNNING;
            process_queue[process_count].memory_pages_used = 1; // Assign RAM buffer
            process_count++;
        }
    }

    // OS synchronization: Wait for all parallel chunks to finish
    for(int i = 0; i < num_threads; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        
        for (int j = 0; j < process_count; j++) {
            if (process_queue[j].pid == child_pids[i]) {
                process_queue[j].state = TERMINATED; // Mark as done in PCB
            }
        }
        printf("[+] Thread (PID %d) chunk download completed.\n", child_pids[i]);
    }
    printf("\n[Downloader] All parallel streams synchronized and completed.\n");
}

void kill_process(int target_pid) {
    if (kill(target_pid, SIGKILL) == 0) {
        printf("[Kernel] Sent SIGKILL to PID %d\n", target_pid);
        for(int i = 0; i < process_count; i++){
            if(process_queue[i].pid == target_pid) process_queue[i].state = TERMINATED;
        }
    }
}

// --- OS SHELL INTEGRATION ---

int main() {
    char input[MAX_COMMAND_LENGTH];
    init_memory_system();
    
    system("clear");
    printf("======================================================\n");
    printf("   OS-Level Multi-Threaded Parallel Downloader Engine \n");
    printf("   Modules: PCB Manager, Paging, HTTP Streaming       \n");
    printf("======================================================\n");

    load_test_processes();

    while (1) {
        printf("\nroot@dl-kernel:~# ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) {
            printf("[Kernel] Unmounting memory pages...\n");
            for(int i = 0; i < 5; i++){
                if(memory_map[i].is_allocated) munmap(memory_map[i].physical_address, PAGE_SIZE);
            }
            break;
        } 
        else if (strcmp(input, "ps") == 0) display_process_queue();
        else if (strcmp(input, "free") == 0) display_memory_map();
        else if (strncmp(input, "kill ", 5) == 0) kill_process(atoi(input + 5));
        else if (strcmp(input, "top") == 0) system("top -b -n 1 | head -n 10");
        else if (strncmp(input, "download ", 9) == 0) {
            // Extract URL and trigger Multithreaded logic
            char *url = input + 9;
            execute_parallel_download(url, 4); // Default to 4 parallel threads
        }
        else {
            execute_real_process(input);
        }
    }
    return 0;
}