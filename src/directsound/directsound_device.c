#include "directsound_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result directsound_deviceGetInfo(Quartz_Device this, Quartz_DeviceInfo *info)
{
	assert(this);
	assert(info);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(info);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_Result directsound_deviceCreateBuffer(Quartz_Device this, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(desc);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_Result directsound_deviceDestroyBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceDestroy(Quartz_Device this)
{
	assert(this);

	DirectSound_Device *ptr = (DirectSound_Device *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result directsound_deviceMapBuffer(Quartz_Device this, Quartz_Buffer buffer, void **ptr)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceUnmapBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_DeviceTable device_vtbl =
{
	directsound_deviceGetInfo,
	directsound_deviceCreateBuffer,

	directsound_deviceDestroyBuffer,
	directsound_deviceDestroy,
	
	directsound_deviceMapBuffer,
	directsound_deviceUnmapBuffer,
};

/*
 */
Quartz_Result directsound_deviceInitialize(DirectSound_Device *device_ptr, DirectSound_Instance *instance_ptr)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data

	return QUARTZ_SUCCESS;
}
