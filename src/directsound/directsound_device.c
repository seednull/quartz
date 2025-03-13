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

static Quartz_Result directsound_deviceDestroy(Quartz_Device this)
{
	assert(this);

	DirectSound_Device *ptr = (DirectSound_Device *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}


/*
 */
static Quartz_DeviceTable device_vtbl =
{
	directsound_deviceGetInfo,
	directsound_deviceDestroy,
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
