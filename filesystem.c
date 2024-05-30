//
// Created by Can Kahraman on 18.01.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "filesystem.h"
#include "disk.h"

char currentDirectory[128];

bool directoryExists(FileSystem *fs, const char *dirname)
{
  for (size_t i = 0; i < fs->disk->nBlocks; ++i)
  {
    if (fs->disk->blocks[i].directories[0].isValid &&
        strcmp(fs->disk->blocks[i].directories[0].dirName, dirname) == 0)
    {
      return true;
    }
  }
  return false;
}

ssize_t findFreeInode(FileSystem *fs)
{
    for (size_t i = 0; i < fs->disk->nBlocks; ++i)
    {
        if (!fs->disk->blocks[i].inodes[0].isValid)
        {
            return i * INODES_PER_BLOCK;
        }
    }
    return -1;
}

ssize_t findFreeDirectoryEntry(FileSystem *fs)
{
    for (size_t i = 0; i < fs->disk->nBlocks; ++i)
    {
        if (!fs->disk->blocks[i].directories[0].isValid)
        {
            return i * INODES_PER_BLOCK;
        }
    }
    return -1;
}

void getFullPath(const char *dirname, const char *filename, char *fullPath, size_t size)
{
    if (dirname != NULL && strcmp(dirname, "/") != 0)
    {
        snprintf(fullPath, size, "%s/%s", dirname, filename);
    }
    else
    {
        strncpy(fullPath, filename, size);
    }
}

ssize_t findINodeByFilename(FileSystem *fs, const char *filename)
{
    for (size_t i = 0; i < fs->disk->nBlocks; ++i)
    {
        if (fs->disk->blocks[i].inodes[0].isValid &&
            strcmp(fs->disk->blocks[i].inodes[0].fileName, filename) == 0)
        {
            return i * INODES_PER_BLOCK;
        }
    }
    return -1;
}

void fsInitialize(FileSystem *fs)
{
  SuperBlock superblock;
  superblock.magicNumber = MAGIC_NUMBER;
  superblock.nBlocks = fs->disk->nBlocks;
  superblock.nINodeBlocks = fs->disk->nBlocks / INODES_PER_BLOCK;
  superblock.nINodes = fs->disk->nBlocks / INODES_PER_BLOCK * INODES_PER_BLOCK;

  diskWrite(0, (char *)&superblock, fs->disk);

  if (!directoryExists(fs, "/"))
  {
    fsCreateDir(fs, "/");
  }

  strncpy(currentDirectory, "/", sizeof(currentDirectory));
}

void fsSync(FileSystem *fs)
{
  for (size_t i = 0; i < fs->disk->nBlocks; ++i)
  {
    diskWrite(i, (char *)&(fs->disk->blocks[i]), fs->disk);
  }
}

void fsLoad(FileSystem *fs)
{
  FILE *file = fopen("disk.img", "rb");
  if (file != NULL)
  {
    fread(fs->disk->blocks, sizeof(struct DataBlock), fs->disk->nBlocks, file);
    fclose(file);
  }
}

void fsSave(FileSystem *fs)
{
  FILE *file = fopen("disk.img", "wb");
  if (file != NULL)
  {
    fwrite(fs->disk->blocks, sizeof(struct DataBlock), fs->disk->nBlocks, file);
    fclose(file);
  }
}

ssize_t fsCreateFile(FileSystem *fs, const char *filename, const char *dirname)
{
  if (dirname != NULL && !directoryExists(fs, dirname))
  {
    printf("Error: Directory '%s' not found\n", dirname);
    return -1;
  }

  char fullFilename[256];
  getFullPath(dirname, filename, fullFilename, sizeof(fullFilename));

  ssize_t inodeNumber = findINodeByFilename(fs, fullFilename);
  if (inodeNumber != -1)
  {
    printf("Error: File '%s' already exists\n", fullFilename);
    return -1;
  }

  inodeNumber = findFreeInode(fs);
  if (inodeNumber != -1)
  {
    fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].isValid = 1;
    fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].size = 0;

    if (dirname != NULL && strcmp(dirname, "/") != 0)
    {
      snprintf(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName,
               sizeof(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName),
               "%s/%s", dirname, filename);
    }
    else
    {
      strncpy(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName, filename,
              sizeof(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName) - 1);
    }

    fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName[sizeof(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].fileName) - 1] = '\0';

    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].createdAt);
    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].accessedAt);
    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].modifiedAt);

    fsSync(fs);

    return inodeNumber;
  }

  return -1;
}

ssize_t fsCreateDir(FileSystem *fs, const char *dirname)
{
  ssize_t directoryInode = findFreeDirectoryEntry(fs);
  if (directoryInode != -1)
  {
    fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].isValid = 1;
    strncpy(fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].dirName,
            dirname, sizeof(fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].dirName) - 1);
    fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].dirName[sizeof(fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].dirName) - 1] = '\0';
    fs->disk->blocks[directoryInode / INODES_PER_BLOCK].directories[0].inodeNumber = (strcmp(dirname, "/") == 0) ? 0 : directoryInode;

    return directoryInode;
  }

  return -1;
}

ssize_t fsWriteFile(FileSystem *fs, const char *filename, const char *data, size_t size, off_t offset)
{
  ssize_t inodeNumber = findINodeByFilename(fs, filename);
  if (inodeNumber != -1)
  {
    size_t blockNumber = offset / BLOCK_SIZE;
    size_t blockOffset = offset % BLOCK_SIZE;

    size_t bytesToWrite = (size < BLOCK_SIZE - blockOffset) ? size : (BLOCK_SIZE - blockOffset);
    memcpy(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].data[blockNumber * BLOCK_SIZE + blockOffset], data, bytesToWrite);

    if (offset + bytesToWrite > fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].size)
    {
      fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].size = offset + bytesToWrite;
    }

    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].modifiedAt);
    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].accessedAt);

    fsSync(fs);
    return bytesToWrite;
  }

  return -1;
}

ssize_t fsReadFile(FileSystem *fs, const char *filename, char *buffer, size_t size, off_t offset)
{
  ssize_t inodeNumber = findINodeByFilename(fs, filename);
  if (inodeNumber != -1)
  {
    size_t blockNumber = offset / BLOCK_SIZE;
    size_t blockOffset = offset % BLOCK_SIZE;

    size_t bytesRead = (size < (size_t)(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].size - offset)) ? size : (size_t)(fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].size - offset);

    memcpy(buffer, &fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].data[blockNumber * BLOCK_SIZE + blockOffset], bytesRead);

    time(&fs->disk->blocks[inodeNumber / INODES_PER_BLOCK].inodes[0].accessedAt);
    return bytesRead;
  }

  return -1;
}

int fsRemoveFile(FileSystem *fs, const char *filename)
{
  for (size_t i = 0; i < fs->disk->nBlocks; ++i)
  {
    if (fs->disk->blocks[i].inodes[0].isValid && strcmp(fs->disk->blocks[i].inodes[0].fileName, filename) == 0)
    {
      fs->disk->blocks[i].inodes[0].isValid = 0;

      fsSync(fs);
      return 0;
    }
  }

  return -1;
}

int fsRemoveDir(FileSystem *fs, const char *dirname)
{
  if (strcmp(dirname, "/") == 0)
  {
    printf("Error: Cannot remove the root directory.\n");
    return -1;
  }

  for (size_t i = 0; i < fs->disk->nBlocks; ++i)
  {
    if (fs->disk->blocks[i].directories[0].isValid && strcmp(fs->disk->blocks[i].directories[0].dirName, dirname) == 0)
    {
      fs->disk->blocks[i].directories[0].isValid = 0;

      for (size_t j = 0; j < fs->disk->nBlocks; ++j)
      {
        if (fs->disk->blocks[j].inodes[0].isValid &&
            strncmp(fs->disk->blocks[j].inodes[0].fileName, dirname, strlen(dirname)) == 0)
        {
          fs->disk->blocks[j].inodes[0].isValid = 0;
        }
      }

      fsSync(fs);
      return 0;
    }
  }

  return -1;
}

void fsChangeDir(FileSystem *fs, const char *dirname)
{
  char newPath[128];

  if (dirname[0] == '/')
  {
    strncpy(newPath, dirname, sizeof(newPath));
  }
  else
  {
    snprintf(newPath, sizeof(newPath), "%s%s%s",
             (strcmp(currentDirectory, "/") == 0) ? "" : currentDirectory,
             (strcmp(currentDirectory, "/") == 0) ? "" : "/",
             dirname);
  }

  if (directoryExists(fs, newPath))
  {
    strncpy(currentDirectory, newPath, sizeof(currentDirectory));
    printf("Changed directory to: %s\n", currentDirectory);
  }
  else
  {
    printf("Error: Directory not found: %s\n", newPath);
  }
}

void fsListEntries(FileSystem *fs, const char *dirname)
{
  if (dirname == NULL || strcmp(dirname, "") == 0)
  {
    dirname = "/";
  }

  strncpy(currentDirectory, dirname, sizeof(currentDirectory));

  if (strcmp(dirname, "/") == 0)
  {
    for (size_t i = 0; i < fs->disk->nBlocks; ++i)
    {
      if (fs->disk->blocks[i].directories[0].isValid && strcmp(fs->disk->blocks[i].directories[0].dirName, dirname) == 0)
      {
        for (size_t j = 0; j < fs->disk->nBlocks; ++j)
        {
          if (fs->disk->blocks[j].directories[0].isValid &&
              ((strcmp(dirname, "/") == 0) ||  // directories in root
              (strncmp(fs->disk->blocks[j].directories[0].dirName, dirname, strlen(dirname)) == 0 &&
                strchr(fs->disk->blocks[j].directories[0].dirName + strlen(dirname), '/') == NULL)))
          {
            if (strcmp(fs->disk->blocks[j].directories[0].dirName, "/") == 0) {
                printf("|-/root (/)\n");
            } else {
                printf(" |-/%s\n", fs->disk->blocks[j].directories[0].dirName);
            }
          }

          if (fs->disk->blocks[j].inodes[0].isValid && // files in root
              ((strcmp(dirname, "/") == 0) && strchr(fs->disk->blocks[j].inodes[0].fileName, '/') == NULL))
          {
            printf(" |-%s\n", fs->disk->blocks[j].inodes[0].fileName);
          }
        }
        return;
      }
    }
  }
  else
  {
    for (size_t i = 0; i < fs->disk->nBlocks; ++i)
    {
      if (fs->disk->blocks[i].directories[0].isValid && strcmp(fs->disk->blocks[i].directories[0].dirName, dirname) == 0)
      {
        printf("|-/%s\n", fs->disk->blocks[i].directories[0].dirName);

        for (size_t j = 0; j < fs->disk->nBlocks; ++j)
        {
          if (fs->disk->blocks[j].inodes[0].isValid &&
              strncmp(fs->disk->blocks[j].inodes[0].fileName, dirname, strlen(dirname)) == 0 &&
              ((strlen(fs->disk->blocks[j].inodes[0].fileName) == strlen(dirname)) ||
               (fs->disk->blocks[j].inodes[0].fileName[strlen(dirname)] == '/')))
          {
            printf(" |-%s\n", strrchr(fs->disk->blocks[j].inodes[0].fileName, '/') + 1);
          }
        }

        return;
      }
    }
  }

  printf("Directory not found: %s\n", dirname);
}

int fsShowInfo(FileSystem *fs, const char *entryname)
{
  for (size_t i = 0; i < fs->disk->nBlocks; ++i)
  {
    if (fs->disk->blocks[i].inodes[0].isValid && strcmp(fs->disk->blocks[i].inodes[0].fileName, entryname) == 0)
    {
      time(&fs->disk->blocks[i].inodes[0].accessedAt);

      printf("File information: \n");
      printf("  Name: %s\n", entryname);
      if (strcmp(entryname, "/") == 0 || strchr(entryname, '/') == NULL)
      {
        printf("  Directory: root\n");
      }
      else
      {
        char dirname[256];
        strncpy(dirname, entryname, sizeof(dirname));
        dirname[strrchr(dirname, '/') - dirname] = '\0';
        printf("  Directory: %s\n", dirname);
      }
      printf("  Inode: %zu\n", i * INODES_PER_BLOCK);
      printf("  Size: %u bytes\n", fs->disk->blocks[i].inodes[0].size);
      printf("  Created At: %s", ctime(&fs->disk->blocks[i].inodes[0].createdAt));
      printf("  Accessed At: %s", ctime(&fs->disk->blocks[i].inodes[0].accessedAt));
      printf("  Modified At: %s\n", ctime(&fs->disk->blocks[i].inodes[0].modifiedAt));

      return 0;
    }
    else if (fs->disk->blocks[i].directories[0].isValid && strcmp(fs->disk->blocks[i].directories[0].dirName, entryname) == 0)
    {
      printf("Directory information: \n");
      printf("  Name: %s\n", entryname);
      printf("  Inode: %u\n", fs->disk->blocks[i].directories[0].inodeNumber);

      return 0;
    }
  }

  return -1;
}
