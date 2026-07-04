#include "webaudio_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

EM_JS(void, js_webaudio_destroyDevice, (int id),
{
	assert(Module);
	assert(Module.quartz.devices.has(id));
	Module.quartz.devices.delete(id);
});

EM_JS(int, js_webaudio_deviceGetSampleRate, (int id),
{
	assert(Module);
	assert(Module.quartz.devices.has(id));

	const device = Module.quartz.devices.get(id);
	assert(device.audio);

	return device.audio.sampleRate;
});

EM_JS(int, js_webaudio_deviceGetChannelCount, (int id),
{
	assert(Module);
	assert(Module.quartz.devices.has(id));

	const device = Module.quartz.devices.get(id);
	assert(device.audio);
	assert(device.audio.destination);

	return device.audio.destination.channelCount;
});

/*
 */
static void webaudio_destroyBuffer(WebAudio_Device *device_ptr, WebAudio_Buffer *buffer_ptr)
{
	QUARTZ_UNUSED(device_ptr);
	QUARTZ_UNUSED(buffer_ptr);
}

static const Quartz_ChannelMapping *webaudio_getChannelMappings(uint32_t channel_count)
{
	static const Quartz_ChannelMapping mono[] =
	{
		QUARTZ_CHANNEL_MAPPING_FRONT_CENTER,
	};

	static const Quartz_ChannelMapping stereo[] =
	{
		QUARTZ_CHANNEL_MAPPING_FRONT_LEFT,
		QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT,
	};

	static const Quartz_ChannelMapping quad[] =
	{
		QUARTZ_CHANNEL_MAPPING_FRONT_LEFT,
		QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT,
		QUARTZ_CHANNEL_MAPPING_BACK_LEFT,
		QUARTZ_CHANNEL_MAPPING_BACK_RIGHT,
	};

	static const Quartz_ChannelMapping surround_5_1[] =
	{
		QUARTZ_CHANNEL_MAPPING_FRONT_LEFT,
		QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT,
		QUARTZ_CHANNEL_MAPPING_FRONT_CENTER,
		QUARTZ_CHANNEL_MAPPING_LFE,
		QUARTZ_CHANNEL_MAPPING_BACK_LEFT,
		QUARTZ_CHANNEL_MAPPING_BACK_RIGHT,
	};

	switch (channel_count)
	{
		case 1: return mono;
		case 2: return stereo;
		case 4: return quad;
		case 6: return surround_5_1;
		default: return NULL;
	}
}

/*
 */
static Quartz_Result webaudio_deviceGetInfo(Quartz_Device this, Quartz_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebAudio_Device *device_ptr = (WebAudio_Device *)this;

	info->api = QUARTZ_API_WEBAUDIO;
	info->type = device_ptr->type;
	memcpy(&info->name[0], device_ptr->info.label, 256);

	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_deviceGetPreferredFormat(Quartz_Device this, Quartz_DeviceFormat *format)
{
	assert(this);
	assert(format);

	WebAudio_Device *device_ptr = (WebAudio_Device *)this;

	const Quartz_ChannelMapping *device_channel_mappings = webaudio_getChannelMappings(device_ptr->channel_count);
	if (!device_channel_mappings)
		return QUARTZ_WEBAUDIO_ERROR;

	memset(format, 0, sizeof(Quartz_DeviceFormat));
	memcpy(format->channel_mappings, device_channel_mappings, sizeof(Quartz_ChannelMapping) * device_ptr->channel_count);
	format->sample_rate = device_ptr->sample_rate;
	format->sample_format = QUARTZ_SAMPLE_FORMAT_FLOAT32;
	format->channel_count = device_ptr->channel_count;

	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_deviceCheckFormatSupport(Quartz_Device this, const Quartz_DeviceFormat *format, uint32_t *supported)
{
	assert(this);
	assert(format);
	assert(supported);

	WebAudio_Device *device_ptr = (WebAudio_Device *)this;

	const Quartz_ChannelMapping *device_channel_mappings = webaudio_getChannelMappings(device_ptr->channel_count);
	if (!device_channel_mappings)
		return QUARTZ_WEBAUDIO_ERROR;

	*supported = 0;

	if (format->sample_rate != device_ptr->sample_rate)
		return QUARTZ_SUCCESS;

	if (format->sample_format != QUARTZ_SAMPLE_FORMAT_FLOAT32)
		return QUARTZ_SUCCESS;

	if (format->channel_count != device_ptr->channel_count)
		return QUARTZ_SUCCESS;

	for (uint32_t i = 0; i < format->channel_count; ++i)
	{
		if (format->channel_mappings[i] != device_channel_mappings[i])
			return QUARTZ_SUCCESS;
	}

	*supported = 1;
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result webaudio_deviceCreateBuffer(Quartz_Device this, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(desc);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_Result webaudio_deviceDestroyBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Quartz_PoolHandle handle = (Quartz_PoolHandle)buffer;
	assert(handle != QUARTZ_POOL_HANDLE_NULL);

	WebAudio_Device *device_ptr = (WebAudio_Device *)this;
	WebAudio_Buffer *buffer_ptr = (WebAudio_Buffer *)quartz_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	quartz_poolRemoveElement(&device_ptr->buffers, handle);

	webaudio_destroyBuffer(device_ptr, buffer_ptr);
	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_deviceDestroy(Quartz_Device this)
{
	assert(this);

	WebAudio_Device *ptr = (WebAudio_Device *)this;

	{
		uint32_t head = quartz_poolGetHeadIndex(&ptr->buffers);
		while (head != QUARTZ_POOL_HANDLE_NULL)
		{
			WebAudio_Buffer *buffer_ptr = (WebAudio_Buffer *)quartz_poolGetElementByIndex(&ptr->buffers, head);
			webaudio_destroyBuffer(ptr, buffer_ptr);

			head = quartz_poolGetNextIndex(&ptr->buffers, head);
		}


		quartz_poolShutdown(&ptr->buffers);
	}

	js_webaudio_destroyDevice(ptr->id);

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result webaudio_deviceStart(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_deviceStop(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_deviceBeginRender(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_deviceEndRender(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_written)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(frames_written);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_deviceBeginCapture(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);
	QUARTZ_UNUSED(frame_count);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result webaudio_deviceEndCapture(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_read)
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
	webaudio_deviceGetInfo,
	webaudio_deviceGetPreferredFormat,
	webaudio_deviceCheckFormatSupport,

	webaudio_deviceCreateBuffer,

	webaudio_deviceDestroyBuffer,
	webaudio_deviceDestroy,
	
	webaudio_deviceStart,
	webaudio_deviceStop,
	webaudio_deviceBeginRender,
	webaudio_deviceEndRender,
	webaudio_deviceBeginCapture,
	webaudio_deviceEndCapture,
};

/*
 */
Quartz_Result webaudio_deviceInitialize(WebAudio_Device *device_ptr, WebAudio_Instance *instance_ptr, Quartz_DeviceType type, const WebAudio_DeviceInfo *info, uint32_t id)
{
	assert(instance_ptr);
	assert(device_ptr);
	assert(info);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->type = type;
	device_ptr->id = id;
	device_ptr->sample_rate = js_webaudio_deviceGetSampleRate(id);
	device_ptr->channel_count = js_webaudio_deviceGetChannelCount(id);
	memcpy(&device_ptr->info, info, sizeof(WebAudio_DeviceInfo));

	// pools
	quartz_poolInitialize(&device_ptr->buffers, sizeof(WebAudio_Buffer), 32);

	return QUARTZ_SUCCESS;
}
