#ifndef HALKERNEL_H
#define HALKERNEL_H

#include <HalSM.h>

void * malloc(UINTN poolSize);
void free(void * pool);
void* realloc(void* ptr,ui64 size);
void* calloc(ui64 num,ui64 size);

#endif