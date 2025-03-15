#include "null_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result null_instanceEnumerateDevices(Quartz_Instance this, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(type);

	*device_count = 1;

	if (infos)
		null_fillDeviceInfo(&infos[0]);

	return QUARTZ_SUCCESS;
}

static Quartz_Result null_instanceCreateDevice(Quartz_Instance this, Quartz_DeviceType type, uint32_t index, Quartz_Device *device)
{
	assert(this);
	assert(device);

	QUARTZ_UNUSED(type);

	if (index != 0)
		return QUARTZ_INVALID_DEVICE_INDEX;

	Null_Instance *instance_ptr = (Null_Instance *)this;
	Null_Device *device_ptr = (Null_Device *)malloc(sizeof(Null_Device));
	assert(device_ptr);

	Quartz_Result result = null_deviceInitialize(device_ptr, instance_ptr);
	if (result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
}

static Quartz_Result null_instanceCreateDefaultDevice(Quartz_Instance this, Quartz_DeviceType type, Quartz_Device *device)
{
	assert(this);
	assert(device);

	QUARTZ_UNUSED(type);

	Null_Instance *instance_ptr = (Null_Instance *)this;
	Null_Device *device_ptr = (Null_Device *)malloc(sizeof(Null_Device));
	assert(device_ptr);

	Quartz_Result result = null_deviceInitialize(device_ptr, instance_ptr);
	if (result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
}

static Quartz_Result null_instanceDestroy(Quartz_Instance this)
{
	assert(this);

	Null_Instance *ptr = (Null_Instance *)this;

	free(ptr->application_name);
	free(ptr->engine_name);

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_InstanceTable instance_vtbl =
{
	null_instanceEnumerateDevices,
	null_instanceCreateDevice,
	null_instanceCreateDefaultDevice,

	null_instanceDestroy,
};

/*
 */
Quartz_Result null_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	Null_Instance *ptr = (Null_Instance *)malloc(sizeof(Null_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// info
	ptr->application_name = strdup(desc->application_name);
	ptr->application_version = desc->application_version;
	ptr->engine_name = strdup(desc->engine_name);
	ptr->engine_version = desc->engine_version;

	*instance = (Quartz_Instance)ptr;
	return QUARTZ_SUCCESS;
}
