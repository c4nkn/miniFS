//
// Created by Can Kahraman on 18.01.2024.
//

#include <stdio.h>
#include "disk.h"

void diskRead(int block_number, char *buffer, struct Disk *disk) {
    FILE *file = fopen("disk.img", "rb");
    fseek(file, block_number * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, file);
    fclose(file);
}

void diskWrite(int block_number, const char *data, struct Disk *disk) {
    FILE *file = fopen("disk.img", "rb+");
    fseek(file, block_number * BLOCK_SIZE, SEEK_SET);
    fwrite(data, BLOCK_SIZE, 1, file);
    fclose(file);
}