#include "webaudio_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result webaudio_instanceEnumerateDevices(Quartz_Instance this, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(type);
	QUARTZ_UNUSED(device_count);
	QUARTZ_UNUSED(infos);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_instanceCreateDevice(Quartz_Instance this, Quartz_DeviceType type, uint32_t index, Quartz_Device *device)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(type);
	QUARTZ_UNUSED(index);
	QUARTZ_UNUSED(device);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_instanceCreateDefaultDevice(Quartz_Instance this, Quartz_DeviceType type, Quartz_Device *device)
{
	return webaudio_instanceCreateDevice(this, type, 0, device);
}

static Quartz_Result webaudio_instanceDestroy(Quartz_Instance this)
{
	assert(this);

	WebAudio_Instance *ptr = (WebAudio_Instance *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_InstanceTable instance_vtbl =
{
	webaudio_instanceEnumerateDevices,
	webaudio_instanceCreateDevice,
	webaudio_instanceCreateDefaultDevice,

	webaudio_instanceDestroy,
};

Quartz_Result webaudio_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	QUARTZ_UNUSED(desc);

	WebAudio_Instance *ptr = (WebAudio_Instance *)malloc(sizeof(WebAudio_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	*instance = (Quartz_Instance)ptr;
	return QUARTZ_SUCCESS;
}
