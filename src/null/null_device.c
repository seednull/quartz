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

static Quartz_Result null_deviceGetPreferredFormat(Quartz_Device this, Quartz_DeviceFormat *format)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceGetCommonFormats(Quartz_Device this, uint32_t *format_count, Quartz_DeviceFormat *formats)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format_count);
	QUARTZ_UNUSED(formats);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceCheckFormatSupport(Quartz_Device this, const Quartz_DeviceFormat *format, uint32_t *supported)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format);
	QUARTZ_UNUSED(supported);

	return QUARTZ_NOT_SUPPORTED;
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
static Quartz_Result null_deviceStart(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceStop(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceBeginRender(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceEndRender(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_written)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(frames_written);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceBeginCapture(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result null_deviceEndCapture(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_read)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(frames_read);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_DeviceTable device_vtbl =
{
	null_deviceGetInfo,
	null_deviceGetPreferredFormat,
	null_deviceGetCommonFormats,
	null_deviceCheckFormatSupport,

	null_deviceCreateBuffer,

	null_deviceDestroyBuffer,
	null_deviceDestroy,
	
	null_deviceStart,
	null_deviceStop,
	null_deviceBeginRender,
	null_deviceEndRender,
	null_deviceBeginCapture,
	null_deviceEndCapture,
};

/*
 */
Quartz_Result null_fillDeviceInfo(Quartz_DeviceType type, Quartz_DeviceInfo *info)
{
	static const char *device_name = "Null Device";

	assert(info);

	memset(info, 0, sizeof(Quartz_DeviceInfo));
	memcpy(info->name, device_name, sizeof(char) * 12);

	info->api = QUARTZ_API_NULL;
	info->type = type;

	return QUARTZ_SUCCESS;
}

Quartz_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr, Quartz_DeviceType type)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	return null_fillDeviceInfo(type, &device_ptr->info);
}
