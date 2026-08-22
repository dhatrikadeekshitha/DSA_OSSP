#!/bin/bash

SOURCE_DIR="mydata"
BACKUP_DIR="backup"

mkdir -p "$SOURCE_DIR"
mkdir -p "$BACKUP_DIR"

echo "Operating Systems Case Study" > "$SOURCE_DIR/file1.txt"
echo "Linux Shell Script Backup" > "$SOURCE_DIR/file2.txt"

BACKUP_FILE="$BACKUP_DIR/backup_$(date +%Y%m%d_%H%M%S).tar.gz"

tar -czf "$BACKUP_FILE" "$SOURCE_DIR"

if [ $? -eq 0 ]; then
    echo "Backup created successfully!"
    echo "Backup file: $BACKUP_FILE"
else
    echo "Backup failed!"
fi
