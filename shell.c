//
// Created by Can Kahraman on 18.01.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "disk.h"

typedef struct DataBlock DataBlock;
typedef struct Disk Disk;

int main() {
    Disk *disk = (Disk *)malloc(sizeof(Disk));
    disk->nBlocks = 1000; 
    disk->blocks = (DataBlock *)malloc(disk->nBlocks * sizeof(DataBlock));

    FileSystem fs;
    fs.disk = disk;
    fsLoad(&fs);
    fsInitialize(&fs);

    if(directoryExists(&fs, "/") == false) {
        fsCreateDir(&fs, "/");
    }

    char input[BUFSIZ], cmd[BUFSIZ], arg1[BUFSIZ], arg2[BUFSIZ], arg3[BUFSIZ];
    char buffer[BLOCK_SIZE];

    printf("Welcome to miniFS!\nMiniFS is a Unix-like structured file system.\nType 'help' for get info about commands.\n\n");

    while (1) {
        printf("miniFS>");
        fgets(input, BUFSIZ, stdin);

        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        memset(arg3, 0, sizeof(arg3));
        
        input[strcspn(input, "\n")] = '\0';
        sscanf(input, "%s %s %s %s", cmd, arg1, arg2, arg3);

        if (strcmp(cmd, "create") == 0) {
            char* dirname = arg1;
            char* filename = arg2;
            ssize_t inumber;

            if (dirname != NULL) {
                inumber = fsCreateFile(&fs, filename, dirname);
                if (inumber != -1) {
                    printf("File '%s' created with inode number %zd in '%s'\n", filename, inumber, dirname);
                } else {
                    printf("Error: Unable to create file '%s' in '%s'\n", filename, dirname);
                }
            } else {
                inumber = fsCreateFile(&fs, filename, "/");
                if (inumber != -1) {
                    printf("File '%s' created with inode number in root('/') directory.%zd\n", filename, inumber);
                } else {
                    printf("Error: Unable to create file '%s' in root('/') directory.\n", filename);
                }
            }
        } else if (strcmp(cmd, "ls") == 0) {
            fsListEntries(&fs, arg1);
        } else if (strcmp(cmd, "cd") == 0) {
            fsChangeDir(&fs, arg1);
        } else if (strcmp(cmd, "write") == 0) {
            ssize_t bytesWritten = fsWriteFile(&fs, arg1, arg2, strlen(arg2), 0);
            if (bytesWritten != -1) {
                printf("%zd bytes written to file: %s\n\n", bytesWritten, arg1);
            } else {
                printf("Error: Unable to write to file '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "read") == 0) {
            memset(buffer, 0, sizeof(buffer));

            ssize_t bytesRead = fsReadFile(&fs, arg1, buffer, BLOCK_SIZE, 0);
            if (bytesRead != -1) {
                printf("Readed %zd bytes:\n %.*s\n", bytesRead, (int)bytesRead, buffer);
            } else {
                printf("Error: Unable to read from file '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "remove") == 0) {
            if (fsRemoveFile(&fs, arg1) == 0) {
                printf("File '%s' removed\n", arg1);
            } else {
                printf("Error: Unable to remove file '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "mkdir") == 0) {
            ssize_t inumber = fsCreateDir(&fs, arg1);
            if (inumber != -1) {
                printf("Directory '%s' created with inode number %zd\n", arg1, inumber);
            } else {
                printf("Error: Unable to create directory '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "rmdir") == 0) {
            if (fsRemoveDir(&fs, arg1) == 0) {
                printf("Directory '%s' removed\n", arg1);
            } else {
                printf("Error: Unable to remove directory '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "info") == 0) {
            if (fsShowInfo(&fs, arg1) == 0) {
            } else {
                printf("Error: Unable to get information for given entry '%s'\n", arg1);
            }
        } else if (strcmp(cmd, "help") == 0) {
            printf("Commands are:\n");
            printf("  -ls <dirname (optional)> : List files and folders in given directory, if null root directory.\n");
            printf("  -open <path/diskname> : Opens image file by specified path and name.\n");
            printf("  -create <dirname> <filename> : Creates file with specified name.\n");
            printf("  -read <dirname/filename> : Reads data of specified file.\n");
            printf("  -write <dirname/filename> <data> : Write data to specified file.\n");
            printf("  -info <dirname/filename> : Shows information about specified file.\n");
            printf("  -remove <dirname/filename> : Removes specified file. \n");
            printf("  -mkdir <dirname> : Creates directory with specified name.\n");
            printf("  -rmdir <dirname> : Removes specified directory. \n");
            printf("  -help : Lists all commands.\n");
            printf("  -exit : Saves changes and closes program.\n\n");
        } else if (strcmp(cmd, "exit") == 0) {
            fsSync(&fs);
            fsSave(&fs);
            break;
        } else {
            printf("Unknown command: %s\nExecute 'help' for list of commands.\n", cmd);
        }
    }

    free(disk->blocks);
    free(disk);
    return EXIT_SUCCESS;
}