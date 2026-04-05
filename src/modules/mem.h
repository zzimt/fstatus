#ifndef FSTATUS_MODULES_MEM_H_
#define FSTATUS_MODULES_MEM_H_

#include <stddef.h>

typedef int MemConfig;

#define Mem_DEFAULT_CONFIG 0

size_t Mem_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_MEM_H_
