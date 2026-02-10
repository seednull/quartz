#pragma once

#include <quartz.h>

#define QUARTZ_POOL_MAX_ELEMENTS		0x00FFFFFF
#define QUARTZ_POOL_MAX_GENERATIONS		0xFF
#define QUARTZ_POOL_HANDLE_NULL			0xFFFFFFFF

typedef uint32_t Quartz_PoolHandle;

typedef struct Quartz_Pool_t
{
	uint8_t *data;
	uint8_t *generations;
	uint32_t *nexts;
	uint32_t *prevs;
	uint32_t head;
	uint32_t tail;

	uint32_t element_size;
	uint32_t size;
	uint32_t capacity;

	uint32_t *masks;
	uint32_t *indices;
	uint32_t num_free_indices;
} Quartz_Pool;

Quartz_Result quartz_poolInitialize(Quartz_Pool *pool, uint32_t element_size, uint32_t capacity);
Quartz_Result quartz_poolShutdown(Quartz_Pool *pool);

Quartz_PoolHandle quartz_poolAddElement(Quartz_Pool *pool, const void *data);
Quartz_Result quartz_poolRemoveElement(Quartz_Pool *pool, Quartz_PoolHandle handle);
void *quartz_poolGetElement(const Quartz_Pool *pool, Quartz_PoolHandle handle);

void *quartz_poolGetElementByIndex(const Quartz_Pool *pool, uint32_t index);
uint32_t quartz_poolGetHeadIndex(const Quartz_Pool *pool);
uint32_t quartz_poolGetTailIndex(const Quartz_Pool *pool);
uint32_t quartz_poolGetNextIndex(const Quartz_Pool *pool, uint32_t index);
uint32_t quartz_poolGetPrevIndex(const Quartz_Pool *pool, uint32_t index);
