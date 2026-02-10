#include "pool.h"
#include "intrinsics.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 */
static QUARTZ_INLINE Quartz_PoolHandle quartz_poolHandlePack(uint32_t index, uint8_t generation)
{
	return (Quartz_PoolHandle)((index << 8) | generation);
}

static QUARTZ_INLINE uint32_t quartz_poolHandleGetIndex(Quartz_PoolHandle handle)
{
	return (uint32_t)(handle >> 8);
}

static QUARTZ_INLINE uint8_t quartz_poolHandleGetGeneration(Quartz_PoolHandle handle)
{
	return (uint8_t)(handle & 0xFF);
}

/*
 */
static QUARTZ_INLINE uint32_t quartz_poolGrabIndex(Quartz_Pool *pool)
{
	assert(pool);
	assert(pool->num_free_indices > 0);

	uint32_t index = pool->indices[pool->num_free_indices - 1];
	pool->num_free_indices--;

	uint32_t mask_index = index / 32;
	uint32_t mask_bit = index % 32;

	pool->masks[mask_index] &= ~(1 << mask_bit);

	return index;
}

static QUARTZ_INLINE void quartz_poolReleaseIndex(Quartz_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(pool->num_free_indices < pool->capacity);
	assert(index < pool->capacity);

	pool->num_free_indices++;
	pool->indices[pool->num_free_indices - 1] = index;

	uint32_t mask_index = index / 32;
	uint32_t mask_bit = index % 32;

	pool->masks[mask_index] |= 1 << mask_bit;
}

static QUARTZ_INLINE uint32_t quartz_poolIsIndexFree(const Quartz_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(index < pool->capacity);

	uint32_t mask_index = index / 32;
	uint32_t mask_bit = index % 32;

	uint32_t free_mask = pool->masks[mask_index];
	uint32_t element_mask = 1 << mask_bit;

	return free_mask & element_mask;
}

static QUARTZ_INLINE uint32_t quartz_poolGetNumMasks(const Quartz_Pool *pool)
{
	assert(pool);

	return alignUp(pool->capacity, 32) / 32;
}

/*
 */
Quartz_Result quartz_poolInitialize(Quartz_Pool *pool, uint32_t element_size, uint32_t capacity)
{
	assert(pool);
	assert(element_size > 0);

	memset(pool, 0, sizeof(Quartz_Pool));

	pool->element_size = element_size;
	pool->capacity = capacity;
	pool->num_free_indices = capacity;
	pool->head = QUARTZ_POOL_HANDLE_NULL;
	pool->tail = QUARTZ_POOL_HANDLE_NULL;

	if (capacity > 0)
	{
		uint32_t num_masks = quartz_poolGetNumMasks(pool);

		pool->data = (uint8_t *)malloc(element_size * capacity);
		pool->generations = (uint8_t *)malloc(sizeof(uint8_t) * capacity);
		pool->nexts = (uint32_t *)malloc(sizeof(uint32_t) * capacity);
		pool->prevs = (uint32_t *)malloc(sizeof(uint32_t) * capacity);
		pool->indices = (uint32_t *)malloc(sizeof(uint32_t) * capacity);
		pool->masks = (uint32_t *)malloc(sizeof(uint32_t) * num_masks);

		for (uint32_t i = 0; i < capacity; ++i)
			pool->indices[i] = capacity - i - 1;

		memset(pool->nexts, QUARTZ_POOL_HANDLE_NULL, sizeof(uint32_t) * capacity);
		memset(pool->prevs, QUARTZ_POOL_HANDLE_NULL, sizeof(uint32_t) * capacity);
		memset(pool->masks, 0xFFFFFFFF, sizeof(uint32_t) * num_masks);
		memset(pool->generations, 0, sizeof(uint8_t) * capacity);
	}

	return QUARTZ_SUCCESS;
}

Quartz_Result quartz_poolShutdown(Quartz_Pool *pool)
{
	assert(pool);

	free(pool->data);
	free(pool->generations);
	free(pool->nexts);
	free(pool->prevs);
	free(pool->indices);
	free(pool->masks);

	memset(pool, 0, sizeof(Quartz_Pool));

	return QUARTZ_SUCCESS;
}

/*
 */
Quartz_PoolHandle quartz_poolAddElement(Quartz_Pool *pool, const void *data)
{
	assert(pool);
	assert(data);

	if (pool->size == pool->capacity)
	{
		uint32_t old_capacity = pool->capacity;
		uint32_t old_num_masks = quartz_poolGetNumMasks(pool);

		pool->capacity = (pool->capacity == 0) ? 1 : pool->capacity * 2;
		uint32_t new_num_masks = quartz_poolGetNumMasks(pool);

		pool->data = (uint8_t *)realloc(pool->data, pool->element_size * pool->capacity);
		pool->generations = (uint8_t *)realloc(pool->generations, sizeof(uint8_t) * pool->capacity);
		pool->nexts = (uint32_t *)realloc(pool->nexts, sizeof(uint32_t) * pool->capacity);
		pool->prevs = (uint32_t *)realloc(pool->prevs, sizeof(uint32_t) * pool->capacity);
		pool->indices = (uint32_t *)realloc(pool->indices, sizeof(uint32_t) * pool->capacity);

		if (old_num_masks != new_num_masks)
			pool->masks = (uint32_t *)realloc(pool->masks, sizeof(uint32_t) * new_num_masks);

		for (uint32_t i = old_capacity; i < pool->capacity; ++i)
		{
			pool->num_free_indices++;
			pool->indices[pool->num_free_indices - 1] = old_capacity + pool->capacity - i - 1;
			pool->generations[i] = 0;
			pool->nexts[i] = QUARTZ_POOL_HANDLE_NULL;
			pool->prevs[i] = QUARTZ_POOL_HANDLE_NULL;
		}

		for (uint32_t i = old_num_masks; i < new_num_masks; ++i)
			pool->masks[i] = 0xFFFFFFFF;
	}

	assert(pool->num_free_indices > 0);

	uint32_t index = quartz_poolGrabIndex(pool);

	uint8_t *data_ptr = pool->data + index * pool->element_size;
	uint8_t *generation_ptr = pool->generations + index;

	if (pool->head == QUARTZ_POOL_HANDLE_NULL)
		pool->head = index;

	if (pool->tail == QUARTZ_POOL_HANDLE_NULL)
	{
		pool->tail = index;
	}
	else
	{
		pool->nexts[pool->tail] = index;
		pool->prevs[index] = pool->tail;

		pool->tail = index;
	}

	memcpy(data_ptr, data, pool->element_size);

	uint8_t generation = *generation_ptr + 1;
	*generation_ptr = max(1, generation);

	pool->size++;

	return quartz_poolHandlePack(index, *generation_ptr);
}

Quartz_Result quartz_poolRemoveElement(Quartz_Pool *pool, Quartz_PoolHandle handle)
{
	assert(pool);

	if (handle == QUARTZ_POOL_HANDLE_NULL)
		return QUARTZ_INTERNAL_ERROR;

	if (pool->size == 0)
		return QUARTZ_INTERNAL_ERROR;

	uint32_t index = quartz_poolHandleGetIndex(handle);
	uint8_t generation = quartz_poolHandleGetGeneration(handle);

	if (pool->generations[index] != generation)
		return QUARTZ_INTERNAL_ERROR;

	if (quartz_poolIsIndexFree(pool, index))
		return QUARTZ_INTERNAL_ERROR;

	uint32_t prev = pool->prevs[index];
	uint32_t next = pool->nexts[index];

	pool->prevs[index] = QUARTZ_POOL_HANDLE_NULL;
	pool->nexts[index] = QUARTZ_POOL_HANDLE_NULL;

	if (next != QUARTZ_POOL_HANDLE_NULL)
		pool->prevs[next] = prev;

	if (prev != QUARTZ_POOL_HANDLE_NULL)
		pool->nexts[prev] = next;

	if (pool->head == index)
		pool->head = next;

	if (pool->tail == index)
		pool->tail = prev;

	quartz_poolReleaseIndex(pool, index);
	pool->size--;

	return QUARTZ_SUCCESS;
}

void *quartz_poolGetElement(const Quartz_Pool *pool, Quartz_PoolHandle handle)
{
	assert(pool);
	assert(pool->size > 0);

	if (handle == QUARTZ_POOL_HANDLE_NULL)
		return NULL;

	uint32_t index = quartz_poolHandleGetIndex(handle);
	uint8_t generation = quartz_poolHandleGetGeneration(handle);

	if (pool->generations[index] != generation)
		return NULL;

	if (quartz_poolIsIndexFree(pool, index))
		return NULL;

	return pool->data + index * pool->element_size;
}

void *quartz_poolGetElementByIndex(const Quartz_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(pool->size > 0);
	assert(pool->capacity > index);
	assert(index != QUARTZ_POOL_HANDLE_NULL);
	
	return pool->data + index * pool->element_size;
}

uint32_t quartz_poolGetHeadIndex(const Quartz_Pool *pool)
{
	assert(pool);
	return pool->head;
}

uint32_t quartz_poolGetTailIndex(const Quartz_Pool *pool)
{
	assert(pool);
	return pool->head;
}

uint32_t quartz_poolGetNextIndex(const Quartz_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(pool->size > 0);
	assert(pool->capacity > index);
	assert(index != QUARTZ_POOL_HANDLE_NULL);
	
	return pool->nexts[index];
}

uint32_t quartz_poolGetPrevIndex(const Quartz_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(pool->size > 0);
	assert(pool->capacity > index);
	assert(index != QUARTZ_POOL_HANDLE_NULL);
	
	return pool->prevs[index];
}
