#include "directsound_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result directsound_instanceEnumerateDevices(Quartz_Instance this, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);
	assert(infos);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(device_count);
	QUARTZ_UNUSED(infos);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_instanceCreateDevice(Quartz_Instance this, uint32_t index, Quartz_Device *device)
{
	assert(this);
	assert(index);
	assert(device);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(index);
	QUARTZ_UNUSED(device);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_instanceDestroy(Quartz_Instance this)
{
	assert(this);

	DirectSound_Instance *ptr = (DirectSound_Instance *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_InstanceTable instance_vtbl =
{
	directsound_instanceEnumerateDevices,
	directsound_instanceCreateDevice,
	directsound_instanceDestroy,
};

/*
 */
Quartz_Result directsound_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	DirectSound_Instance *ptr = (DirectSound_Instance *)malloc(sizeof(DirectSound_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// info

	*instance = (Quartz_Instance)ptr;
	return QUARTZ_SUCCESS;
}
