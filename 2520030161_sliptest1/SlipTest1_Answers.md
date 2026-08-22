# Slip Test 1 - 18 August 2026

**Register Number:** 2520030161

## Question 1 - Creating a New Process

### 1A. Arrange the major components involved in executing fork()

The order is:

User Application
↓
C Library / System Call Interface
↓
fork() System Call
↓
Linux Kernel
↓
Process Management Subsystem
↓
CPU
↓
Memory and Hardware Resources

The user application calls fork(). The request is passed through the system-call interface to the Linux kernel. The kernel's process-management subsystem creates and initializes the child process. The CPU executes the required instructions and memory and other hardware resources are used by the new process.

### 1B. Investigation plan for fork()

A Linux system-monitoring tool such as strace can be used.

1. Create a C program containing fork().
2. Compile it using:
   gcc fork.c -o fork
3. Run it using:
   strace ./fork
4. Observe the process-creation system call such as fork(), clone(), or clone3(), depending on the Linux implementation.
5. The system call causes a transition from User Mode to Kernel Mode.
6. The Linux kernel process-management subsystem creates the child process.
7. Successful process creation can be verified using the returned process ID and commands such as:
   ps
   pstree

Therefore, system-call tracing and process-monitoring tools can confirm that the child process was successfully created.

### 1C. Restricting unlimited process creation

Linux can restrict unlimited process creation using process and resource limits. User limits can restrict the maximum number of processes a user can create. Resource limits and control groups can prevent one user or group of processes from consuming all system resources. Linux permissions and privileges also restrict unauthorized operations.

Therefore, Linux process-management and protection mechanisms can prevent a malicious program from exhausting system resources by creating unlimited processes.

## Question 4 - Creating a File

### 4A. Arrange the major components involved in creating assignment.txt

The order is:

User
↓
Shell
↓
touch command
↓
C Library / System Call Interface
↓
System Call
↓
Linux Kernel
↓
Virtual File System (VFS)
↓
File System
↓
Storage Driver
↓
Storage Hardware

The user enters touch assignment.txt in the shell. The touch program requests the file operation through the system-call interface. The Linux kernel processes the request. The VFS and appropriate file-system layer manage the file, and the storage driver communicates with the storage hardware.

### 4B. Investigation plan using a system-call tracing tool

A system-call tracing tool such as strace can be used.

Run:

strace touch assignment.txt

The investigation identifies:

1. User-space program:
   touch

2. System call:
   A system call such as openat() may be used with creation flags depending on the implementation.

3. User Mode to Kernel Mode:
   When touch invokes a system call, execution changes from User Mode to Kernel Mode.

4. Kernel file-system service:
   The Linux Virtual File System (VFS) and file-system subsystem process the request.

5. Metadata:
   File name, file type, permissions, owner, group, timestamps, and inode information may be created or updated.

The result can be verified using:

ls -l assignment.txt

and:

stat assignment.txt

### 4C. Preventing unauthorized file creation

Linux uses permissions and ownership to prevent a student from creating a file in another user's directory.

The kernel checks the owner, group, other-user permissions, and the permissions of the directory. A user normally needs appropriate write and search/execute permission on the directory to create a file inside it.

If the required permissions are absent, the kernel denies the operation and the file is not created.

Therefore, Linux permissions and ownership mechanisms protect users from unauthorized file creation or modification.
