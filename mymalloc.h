#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<pthread.h>

typedef struct s_block Block;

void* mymalloc(size_t size);

void myfree(void *ptr);

void* mycalloc(size_t num, size_t size);

void* myrealloc(void *ptr, size_t size);

void list_blocks();

#endif
