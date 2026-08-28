# Practical 11 - Inodes, Links and Memory-Mapped I/O

## Aim

To investigate Linux inode structures using `ls -i`, `stat`, and `find`, create hard and symbolic links, and study their effect on inode allocation. Also, to implement file reading and writing using `mmap()` and compare it with traditional `read()` and `write()` operations.

---

# Part A - Inode Investigation

## Inode

An inode is a filesystem data structure that stores metadata about a file.

It contains information such as:

- File type
- Permissions
- Owner
- File size
- Timestamps
- Number of hard links
- Pointers to file data blocks

The filename itself is stored in a directory entry that refers to the inode.

## Commands Used

### ls -i

```bash
ls -i inode_test.txt
