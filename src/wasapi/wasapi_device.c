#include "WASAPI_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result wasapi_deviceGetInfo(Quartz_Device this, Quartz_DeviceInfo *info)
{
	assert(this);
	assert(info);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(info);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result wasapi_deviceDestroy(Quartz_Device this)
{
	assert(this);

	WASAPI_Device *ptr = (WASAPI_Device *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}


/*
 */
static Quartz_DeviceTable device_vtbl =
{
	wasapi_deviceGetInfo,
	wasapi_deviceDestroy,
};

/*
 */
Quartz_Result wasapi_deviceInitialize(WASAPI_Device *device_ptr, WASAPI_Instance *instance_ptr)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data

	return QUARTZ_SUCCESS;
}
