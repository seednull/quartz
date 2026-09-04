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

static Quartz_Result directsound_deviceGetPreferredFormat(Quartz_Device this, Quartz_DeviceFormat *format)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceCheckFormatSupport(Quartz_Device this, const Quartz_DeviceFormat *format, uint32_t *supported)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format);
	QUARTZ_UNUSED(supported);

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
static Quartz_Result directsound_deviceStart(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceStop(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceBeginRender(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceEndRender(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_written)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(frames_written);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceBeginCapture(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result directsound_deviceEndCapture(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_read)
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
	directsound_deviceGetInfo,
	directsound_deviceGetPreferredFormat,
	directsound_deviceCheckFormatSupport,

	directsound_deviceCreateBuffer,

	directsound_deviceDestroyBuffer,
	directsound_deviceDestroy,
	
	directsound_deviceStart,
	directsound_deviceStop,
	directsound_deviceBeginRender,
	directsound_deviceEndRender,
	directsound_deviceBeginCapture,
	directsound_deviceEndCapture,
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
