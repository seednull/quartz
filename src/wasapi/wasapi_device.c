#include "wasapi_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct WASAPI_Channel_t
{
	DWORD speaker;
	Quartz_ChannelMapping mapping;
} WASAPI_Channel;

static WASAPI_Channel channel_map[] =
{
	{ SPEAKER_FRONT_LEFT, QUARTZ_CHANNEL_MAPPING_FRONT_LEFT },
	{ SPEAKER_FRONT_RIGHT, QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT },
	{ SPEAKER_FRONT_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_CENTER },
	{ SPEAKER_LOW_FREQUENCY, QUARTZ_CHANNEL_MAPPING_LFE },
	{ SPEAKER_BACK_LEFT, QUARTZ_CHANNEL_MAPPING_BACK_LEFT },
	{ SPEAKER_BACK_RIGHT, QUARTZ_CHANNEL_MAPPING_BACK_RIGHT },
	{ SPEAKER_FRONT_LEFT_OF_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_LEFT_OF_CENTER },
	{ SPEAKER_FRONT_RIGHT_OF_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT_OF_CENTER },
	{ SPEAKER_BACK_CENTER, QUARTZ_CHANNEL_MAPPING_BACK_CENTER },
	{ SPEAKER_SIDE_LEFT, QUARTZ_CHANNEL_MAPPING_SIDE_LEFT },
	{ SPEAKER_SIDE_RIGHT, QUARTZ_CHANNEL_MAPPING_SIDE_RIGHT },
	{ SPEAKER_TOP_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_CENTER },
	{ SPEAKER_TOP_FRONT_LEFT, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_LEFT },
	{ SPEAKER_TOP_FRONT_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_CENTER },
	{ SPEAKER_TOP_FRONT_RIGHT, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_RIGHT },
	{ SPEAKER_TOP_BACK_LEFT, QUARTZ_CHANNEL_MAPPING_TOP_BACK_LEFT },
	{ SPEAKER_TOP_BACK_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_BACK_CENTER },
	{ SPEAKER_TOP_BACK_RIGHT, QUARTZ_CHANNEL_MAPPING_TOP_BACK_RIGHT },
};

static GUID subtype_pcm = { STATIC_KSDATAFORMAT_SUBTYPE_PCM };
static GUID subtype_ieee_float = { STATIC_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT };

/*
 */
static void wasapi_destroyBuffer(WASAPI_Device *device_ptr, WASAPI_Buffer *buffer_ptr)
{
	QUARTZ_UNUSED(device_ptr);
	assert(buffer_ptr);

	if (buffer_ptr->capture_client)
		IAudioCaptureClient_Release(buffer_ptr->capture_client);

	if (buffer_ptr->render_client)
		IAudioRenderClient_Release(buffer_ptr->render_client);

	IAudioClient_Release(buffer_ptr->client);
}

/*
 */
static Quartz_Result wasapi_deviceGetInfo(Quartz_Device this, Quartz_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;
	IMMDevice *wasapi_device = device_ptr->device;

	return wasapi_helperFillDeviceInfo(wasapi_device, device_ptr->type, info);
}

static Quartz_Result wasapi_deviceGetPreferredFormat(Quartz_Device this, Quartz_DeviceFormat *format)
{
	assert(this);
	assert(format);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;
	IMMDevice *wasapi_device = device_ptr->device;

	IAudioClient *wasapi_client = NULL;
	HRESULT hr = IMMDevice_Activate(wasapi_device, &IID_IAudioClient, CLSCTX_ALL, NULL, &wasapi_client);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	WAVEFORMATEXTENSIBLE *wasapi_format = NULL;
	hr = IAudioClient_GetMixFormat(wasapi_client, (WAVEFORMATEX **)&wasapi_format);
	IAudioClient_Release(wasapi_client);

	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	memset(format, 0, sizeof(Quartz_DeviceFormat));
	format->sample_rate = wasapi_format->Format.nSamplesPerSec;
	format->channel_count = wasapi_format->Format.nChannels;

	WORD bit_depth = wasapi_format->Format.wBitsPerSample;
	WORD tag = wasapi_format->Format.wFormatTag;

	if (tag == WAVE_FORMAT_EXTENSIBLE)
	{
		bit_depth = wasapi_format->Samples.wValidBitsPerSample;

		if (IsEqualGUID(&wasapi_format->SubFormat, &subtype_pcm))
			tag = WAVE_FORMAT_PCM;

		if (IsEqualGUID(&wasapi_format->SubFormat, &subtype_ieee_float))
			tag = WAVE_FORMAT_IEEE_FLOAT;

		DWORD mask = wasapi_format->dwChannelMask;
		uint32_t index = 0;
		for (uint32_t i = 0; i < ARRAYSIZE(channel_map); ++i)
		{
			if (mask & channel_map[i].speaker)
			{
				format->channel_mappings[index++] = channel_map[i].mapping;
			}
		}

		assert(index == format->channel_count);
	}

	if (tag == WAVE_FORMAT_PCM)
	{
		switch (bit_depth)
		{
			case 8: format->sample_format = QUARTZ_SAMPLE_FORMAT_UINT8; break;
			case 16: format->sample_format = QUARTZ_SAMPLE_FORMAT_SINT16; break;
			case 24: format->sample_format = QUARTZ_SAMPLE_FORMAT_SINT24; break;
			case 32: format->sample_format = QUARTZ_SAMPLE_FORMAT_SINT32; break;
			default: format->sample_format = QUARTZ_SAMPLE_FORMAT_UNKNOWN; break;
		}
	}
	else if (tag == WAVE_FORMAT_IEEE_FLOAT)
	{
		switch (bit_depth)
		{
			case 32: format->sample_format = QUARTZ_SAMPLE_FORMAT_FLOAT32; break;
			default: format->sample_format = QUARTZ_SAMPLE_FORMAT_UNKNOWN; break;
		}
	}

	CoTaskMemFree(wasapi_format);
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceGetSupportedFormats(Quartz_Device this, uint32_t *format_count, Quartz_DeviceFormat *formats)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(format_count);
	QUARTZ_UNUSED(formats);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_Result wasapi_deviceCreateBuffer(Quartz_Device this, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer)
{
	assert(this);
	assert(desc);
	assert(buffer);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;
	IMMDevice *wasapi_device = device_ptr->device;

	// initialize
	IAudioClient *wasapi_client = NULL;
	HRESULT hr = IMMDevice_Activate(wasapi_device, &IID_IAudioClient, CLSCTX_ALL, NULL, &wasapi_client);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	AUDCLNT_SHAREMODE share_mode = AUDCLNT_SHAREMODE_SHARED;
	REFERENCE_TIME duration = desc->duration_milliseconds * 10000;

	uint32_t container_bit_depth = wasapi_helperToContainerBitDepth(desc->format.sample_format);
	uint32_t audio_frame_size = desc->format.channel_count * container_bit_depth / 8;

	WAVEFORMATEXTENSIBLE format = {0};
	format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	format.Format.nChannels = (WORD)desc->format.channel_count;
	format.Format.nSamplesPerSec = desc->format.sample_rate;
	format.Format.nAvgBytesPerSec = desc->format.sample_rate * audio_frame_size;
	format.Format.nBlockAlign = (WORD)audio_frame_size;
	format.Format.wBitsPerSample = (WORD)container_bit_depth;
	format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

	format.Samples.wValidBitsPerSample = (WORD)wasapi_helperToActualBitDepth(desc->format.sample_format);
	for (uint32_t i = 0; i < desc->format.channel_count; ++i)
		format.dwChannelMask |= wasapi_helperToSpeakerMask(desc->format.channel_mappings[i]);

	format.SubFormat = wasapi_helperToSubFormat(desc->format.sample_format);

	WAVEFORMATEXTENSIBLE *closest_match_format = NULL;
	hr = IAudioClient_IsFormatSupported(wasapi_client, share_mode, (const WAVEFORMATEX *)&format, (WAVEFORMATEX **)&closest_match_format);
	if (!SUCCEEDED(hr))
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_WASAPI_ERROR;
	}

	if (hr == S_FALSE)
	{
		CoTaskMemFree(closest_match_format);
		IAudioClient_Release(wasapi_client);
		return QUARTZ_INVALID_BUFFER_FORMAT;
	}

	hr = IAudioClient_Initialize(wasapi_client, share_mode, 0, duration, 0, (const WAVEFORMATEX *)&format, NULL);
	if (!SUCCEEDED(hr))
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_WASAPI_ERROR;
	}

	UINT32 wasapi_buffer_size = 0;
	hr = IAudioClient_GetBufferSize(wasapi_client, &wasapi_buffer_size);
	if (!SUCCEEDED(hr))
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_WASAPI_ERROR;
	}

	IAudioRenderClient *wasapi_render_client = NULL;
	IAudioCaptureClient *wasapi_capture_client = NULL;

	uint32_t want_render = (device_ptr->type == QUARTZ_DEVICE_TYPE_RENDER);
	uint32_t want_capture = (device_ptr->type == QUARTZ_DEVICE_TYPE_CAPTURE);

	if (want_render)
	{
		hr = IAudioClient_GetService(wasapi_client, &IID_IAudioRenderClient, &wasapi_render_client);
		if (!SUCCEEDED(hr))
		{
			IAudioClient_Release(wasapi_client);
			return QUARTZ_WASAPI_ERROR;
		}
	}

	if (want_capture)
	{
		hr = IAudioClient_GetService(wasapi_client, &IID_IAudioCaptureClient, &wasapi_capture_client);
		if (!SUCCEEDED(hr))
		{
			IAudioClient_Release(wasapi_client);
			return QUARTZ_WASAPI_ERROR;
		}
	}

	// create quartz struct
	WASAPI_Buffer result = {0};
	result.client = wasapi_client;
	result.render_client = wasapi_render_client;
	result.capture_client = wasapi_capture_client;
	result.size = wasapi_buffer_size;

	*buffer = (Quartz_Buffer)quartz_poolAddElement(&device_ptr->buffers, &result);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result wasapi_deviceDestroyBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Quartz_PoolHandle handle = (Quartz_PoolHandle)buffer;
	assert(handle != QUARTZ_POOL_HANDLE_NULL);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;
	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	quartz_poolRemoveElement(&device_ptr->buffers, handle);

	wasapi_destroyBuffer(device_ptr, buffer_ptr);
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceDestroy(Quartz_Device this)
{
	assert(this);

	WASAPI_Device *ptr = (WASAPI_Device *)this;

	{
		uint32_t head = quartz_poolGetHeadIndex(&ptr->buffers);
		while (head != QUARTZ_POOL_HANDLE_NULL)
		{
			WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElementByIndex(&ptr->buffers, head);
			wasapi_destroyBuffer(ptr, buffer_ptr);

			head = quartz_poolGetNextIndex(&ptr->buffers, head);
		}


		quartz_poolShutdown(&ptr->buffers);
	}

	IMMDevice_Release(ptr->device);

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result wasapi_deviceStart(Quartz_Device this, Quartz_Buffer buffer)
{
	assert(this);
	assert(buffer);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);

	HRESULT hr = IAudioClient_Start(buffer_ptr->client);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceStop(Quartz_Device this, Quartz_Buffer buffer)
{
	assert(this);
	assert(buffer);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);

	HRESULT hr = IAudioClient_Stop(buffer_ptr->client);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceBeginRender(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	assert(this);
	assert(buffer);
	assert(ptr);
	assert(frame_count);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);
	assert(buffer_ptr->render_client);

	UINT32 padding = 0;
	HRESULT hr = IAudioClient_GetCurrentPadding(buffer_ptr->client, &padding);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	UINT32 frames = buffer_ptr->size - padding;

	hr = IAudioRenderClient_GetBuffer(buffer_ptr->render_client, frames, (BYTE **)ptr);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	*frame_count = frames;
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceEndRender(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_written)
{
	assert(this);
	assert(buffer);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);
	assert(buffer_ptr->render_client);

	// TODO: expose silent flag in the API?
	HRESULT hr = IAudioRenderClient_ReleaseBuffer(buffer_ptr->render_client, frames_written, 0);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceBeginCapture(Quartz_Device this, Quartz_Buffer buffer, void **ptr, uint32_t *frame_count)
{
	assert(this);
	assert(buffer);
	assert(ptr);
	assert(frame_count);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);
	assert(buffer_ptr->capture_client);

	// TODO: should we zero buffer manually or expose silent flag in the API?
	DWORD flags = 0;

	HRESULT hr = IAudioCaptureClient_GetBuffer(buffer_ptr->capture_client, (BYTE **)ptr, frame_count, &flags, NULL, NULL);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceEndCapture(Quartz_Device this, Quartz_Buffer buffer, uint32_t frames_read)
{
	assert(this);
	assert(buffer);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;

	WASAPI_Buffer *buffer_ptr = (WASAPI_Buffer *)quartz_poolGetElement(&device_ptr->buffers, (Quartz_PoolHandle)buffer);
	assert(buffer_ptr);
	assert(buffer_ptr->client);
	assert(buffer_ptr->capture_client);

	HRESULT hr = IAudioCaptureClient_ReleaseBuffer(buffer_ptr->capture_client, frames_read);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;
	
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_DeviceTable device_vtbl =
{
	wasapi_deviceGetInfo,
	wasapi_deviceGetPreferredFormat,
	wasapi_deviceGetSupportedFormats,

	wasapi_deviceCreateBuffer,

	wasapi_deviceDestroyBuffer,
	wasapi_deviceDestroy,
	
	wasapi_deviceStart,
	wasapi_deviceStop,
	wasapi_deviceBeginRender,
	wasapi_deviceEndRender,
	wasapi_deviceBeginCapture,
	wasapi_deviceEndCapture,
};

/*
 */
Quartz_Result wasapi_deviceInitialize(WASAPI_Device *device_ptr, WASAPI_Instance *instance_ptr, IMMDevice *wasapi_device, Quartz_DeviceType type)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->type = type;
	device_ptr->device = wasapi_device;

	// pools
	quartz_poolInitialize(&device_ptr->buffers, sizeof(WASAPI_Buffer), 32);

	return QUARTZ_SUCCESS;
}
