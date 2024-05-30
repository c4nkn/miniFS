//
// Created by Can Kahraman on 18.01.2024.
//

#ifndef MINIFS_DISK_H
#define MINIFS_DISK_H

#include "stdlib.h"

#define BLOCK_SIZE 4096

typedef struct DataBlock DataBlock_t;

typedef struct Disk
{
    struct DataBlock *blocks;
    size_t nBlocks;
} Disk_t;

void diskRead(int block_number, char *buffer, struct Disk *disk);
void diskWrite(int block_number, const char *data, struct Disk *disk);

#endif //MINIFS_DISK_H
