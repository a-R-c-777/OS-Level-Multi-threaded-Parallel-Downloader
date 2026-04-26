# OS-Level-Multi-threaded-Parallel-Downloader

```markdown
# KOS-DOWNLOADER: Kernel-Level Parallel Download Engine

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL-lightgrey.svg)
![Concepts](https://img.shields.io/badge/Concepts-POSIX%20%7C%20Paging-success.svg)

## 📌 Overview
KOS-DOWNLOADER is a specialized, C-based mini-operating system kernel engineered specifically to handle high-speed, multithreaded parallel file downloads. 

Unlike standard user-space download scripts, this engine interacts directly with the Linux kernel's Virtual Memory Manager and CPU Scheduler. It demonstrates advanced Operating System concepts by utilizing POSIX system calls to manage concurrent download threads and allocate non-fragmenting memory buffers.

This project was developed collaboratively as an academic engineering study, split into two core modules: **Process Scheduling & CPU Management** and **Memory & Resource Management**.

---

## ⚙️ Core Features

* **Parallel HTTP Streaming:** Splits massive files into 1MB chunks and downloads them concurrently to maximize NIC bandwidth.
* **Custom CLI Shell:** A built-in terminal interface running an infinite command-polling loop.
* **Process Control Block (PCB) Management:** Dynamically tracks the PIDs, execution states (READY, DOWNLOADING, TERMINATED), and memory usage of all active background and foreground threads.
* **Anonymous Memory Paging:** Bypasses standard `malloc()` by using `mmap()` to allocate 4KB fixed-size physical memory frames directly from the kernel, preventing memory fragmentation during high-speed I/O.
* **Zombie Process Prevention:** Strict thread synchronization utilizing `waitpid()` to ensure safe CPU scheduling and context switching.
* **Hardware Telemetry:** Real-time polling of host CPU load and RAM utilization.

---

## 🧠 System Architecture

### 1. Process & CPU Manager Atharva
Responsible for the process lifecycle. When a download is initiated, the engine utilizes `fork()` to spawn independent execution branches and `execvp()` to assign them network utility tasks. It handles process synchronization, state transitions, and asynchronous hardware interrupts via `SIGKILL`.

### 2. Memory & Buffer Manager Vedant
Responsible for RAM allocation. Implements a logical-to-physical Page Table. It utilizes `mmap()` with `MAP_ANONYMOUS | MAP_PRIVATE` flags to grant threads secure RAM buffers for incoming network packets, and strictly enforces `munmap()` upon termination to guarantee zero memory leaks.

---

## 🚀 Installation & Setup

### Prerequisites
* A Linux environment (Native Linux or Windows Subsystem for Linux - WSL)
* GNU C Compiler (`gcc`)
* `curl` (Network utility package)

### Build Instructions
1. Clone the repository:
   ```bash
   git clone [https://github.com/yourusername/Parallel_Downloader.git](https://github.com/yourusername/Parallel_Downloader.git)
   cd Parallel_Downloader
   ```
2. Compile the kernel:
   ```bash
   gcc parallel_downloader_os.c -o downloader_kernel
   ```
3. Boot the engine:
   ```bash
   ./downloader_kernel
   ```

---

## 💻 Usage & Commands

Once the KOS-DOWNLOADER shell is active (`root@dl-kernel:~#`), you can use the following commands:

| Command | Description |
| :--- | :--- |
| `download [URL]` | **(Main Event)** Forks parallel threads to download the specified file URL in chunks. |
| `ps` | Displays the current Process Control Block (PCB) table and thread states. |
| `free` | Displays the physical memory buffer map and `mmap` allocations. |
| `top` | Polls the host OS for real-time CPU and RAM hardware telemetry. |
| `kill [PID]` | Sends a `SIGKILL` hardware interrupt to immediately halt a specific thread. |
| `exit` | Safely unmounts all memory pages (`munmap`) and halts the kernel. |

---

## 👥 Contributors
* **Atharva Chivate** - *Process Scheduling & CPU Management* * **Vedant Dakhore** - *Memory Allocation & Buffer Management*

*Developed as a Project: Multithreaded Parallel Downloader for Vishwakarma Institute of Technology, Operating Systems Course.*
```
