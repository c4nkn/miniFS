//
// Created by Can Kahraman on 18.01.2024.
//

#ifndef MINIFS_FILESYSTEM_H
#define MINIFS_FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#define BLOCK_SIZE 4096
#define MAGIC_NUMBER 0xf0f03410
#define POINTERS_FOR_INODE 5
#define POINTERS_PER_BLOCK 1024
#define INODES_PER_BLOCK 128
#define MAX_FILES_PER_DIR 128

typedef struct Disk disk_t;

typedef struct
{
    struct Disk *disk;
} FileSystem;

typedef struct 
{
    uint32_t magicNumber;
    uint32_t nBlocks;
    uint32_t nINodeBlocks;
    uint32_t nINodes;
} __attribute__((packed)) SuperBlock;

typedef struct
{
    uint32_t isValid;
    uint32_t size;
    char fileName[256];
    uint32_t direct[POINTERS_FOR_INODE];
    uint32_t indirect;
    time_t createdAt;
    time_t accessedAt;
    time_t modifiedAt;
} __attribute__((packed)) INode;

typedef struct
{
    uint32_t isValid;
    char dirName[256];
    uint32_t inodeNumber;
} __attribute__((packed)) DirectoryEntry;

typedef struct DataBlock
{
    SuperBlock superblock;
    INode inodes[INODES_PER_BLOCK];
    DirectoryEntry directories[MAX_FILES_PER_DIR];
    uint32_t pointers[POINTERS_PER_BLOCK];
    char data[BLOCK_SIZE];
} __attribute__((packed)) DataBlock_t;


bool directoryExists(FileSystem *fs, const char *dirname);

void fsInitialize(FileSystem *fs);
void fsLoad(FileSystem *fs);
void fsSync(FileSystem *fs);
void fsSave(FileSystem *fs);

ssize_t fsCreateFile(FileSystem *fs, const char *filename, const char *dirname);
ssize_t fsCreateDir(FileSystem *fs, const char *dirname);
ssize_t fsWriteFile(FileSystem *fs, const char *filename, const char *data, size_t size, off_t offset);
ssize_t fsReadFile(FileSystem *fs, const char *filename, char *buffer, size_t size, off_t offset);

int fsRemoveFile(FileSystem *fs, const char *filename);
int fsRemoveDir(FileSystem *fs, const char *dirname);
int fsShowInfo(FileSystem *fs, const char *entryname);

void fsChangeDir(FileSystem *fs, const char *dirname);
void fsListEntries(FileSystem *fs, const char *dirname);

#endif //MINIFS_FILESYSTEM_H
