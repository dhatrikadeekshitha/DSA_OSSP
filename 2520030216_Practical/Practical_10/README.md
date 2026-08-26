# Practical 10 - Linux File I/O and I/O Redirection

## Aim

To implement a file copy utility using low-level Linux file I/O system calls and compare its performance with standard C library file functions. Also, to demonstrate standard input and output redirection using file descriptors and dup2().

---

## Part A - File Copy Utility

The program `file_copy.c` implements file copying using low-level Linux system calls:

- open()
- read()
- write()
- lseek()
- close()

It also implements file copying using standard library functions:

- fopen()
- fread()
- fwrite()
- fclose()

The execution time of both methods is measured using `gettimeofday()`.

### Low-Level File I/O

The low-level implementation directly communicates with the Linux kernel using file descriptors.

### Standard Library I/O

The standard library implementation uses buffered FILE streams provided by the C standard library.

### Performance Comparison

The program measures the execution time of both approaches.

The actual performance depends on factors such as:

- File size
- Buffer size
- Filesystem
- Operating system caching
- System load

Therefore, a single execution should not be used to conclude that one method is always faster.

### Verification

The copied files can be compared using:

```bash
cmp testfile.txt copy_lowlevel.txt
