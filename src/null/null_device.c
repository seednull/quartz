#include "null_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result null_deviceGetInfo(Quartz_Device this, Quartz_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Null_Device *ptr = (Null_Device *)this;

	memcpy(info, &ptr->info, sizeof(Quartz_DeviceInfo));
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result null_deviceCreateBuffer(Quartz_Device this, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(desc);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_Result null_deviceDestroyBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceDestroy(Quartz_Device this)
{
	assert(this);

	Null_Device *ptr = (Null_Device *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result null_deviceMapBuffer(Quartz_Device this, Quartz_Buffer buffer, void **ptr)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceUnmapBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_DeviceTable device_vtbl =
{
	null_deviceGetInfo,
	null_deviceCreateBuffer,

	null_deviceDestroyBuffer,
	null_deviceDestroy,
	
	null_deviceMapBuffer,
	null_deviceUnmapBuffer,
};

/*
 */
Quartz_Result null_fillDeviceInfo(Quartz_DeviceInfo *info)
{
	static const char *device_name = "Null Device";

	assert(info);

	memset(info, 0, sizeof(Quartz_DeviceInfo));
	memcpy(info->name, device_name, sizeof(char) * 12);

	info->api = QUARTZ_API_NULL;

	return QUARTZ_SUCCESS;
}

Quartz_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	return null_fillDeviceInfo(&device_ptr->info);
}
