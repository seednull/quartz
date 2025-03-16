#include "WASAPI_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static void wasapi_destroyBuffer(WASAPI_Device *device_ptr, WASAPI_Buffer *buffer_ptr)
{
	QUARTZ_UNUSED(device_ptr);
	assert(buffer_ptr);

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

	return wasapi_helperFillDeviceInfo(wasapi_device, info);
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
	REFERENCE_TIME duration = desc->duration_milliseconds * 1000;

	uint32_t bit_depth = wasapi_helperToBitDepth(desc->format);
	uint32_t audio_frame_size = desc->num_channels * bit_depth / 8;

	WAVEFORMATEX format = {0};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (WORD)desc->num_channels;
	format.nSamplesPerSec = desc->sample_rate;
	format.nAvgBytesPerSec = desc->sample_rate * audio_frame_size;
	format.nBlockAlign = (WORD)audio_frame_size;
	format.wBitsPerSample = (WORD)bit_depth;

	WAVEFORMATEX *closest_match_format = NULL;
	hr = IAudioClient_IsFormatSupported(wasapi_client, share_mode, &format, &closest_match_format);
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

	hr = IAudioClient_Initialize(wasapi_client, share_mode, 0, duration, 0, &format, NULL);
	if (!SUCCEEDED(hr))
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_WASAPI_ERROR;
	}

	// create quartz struct
	WASAPI_Buffer result = {0};
	result.client = wasapi_client;

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
static Quartz_Result wasapi_deviceMapBuffer(Quartz_Device this, Quartz_Buffer buffer, void **ptr)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);
	QUARTZ_UNUSED(ptr);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result wasapi_deviceUnmapBuffer(Quartz_Device this, Quartz_Buffer buffer)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(buffer);

	return QUARTZ_NOT_SUPPORTED;
}

/*
 */
static Quartz_DeviceTable device_vtbl =
{
	wasapi_deviceGetInfo,
	wasapi_deviceCreateBuffer,

	wasapi_deviceDestroyBuffer,
	wasapi_deviceDestroy,
	
	wasapi_deviceMapBuffer,
	wasapi_deviceUnmapBuffer,
};

/*
 */
Quartz_Result wasapi_deviceInitialize(WASAPI_Device *device_ptr, WASAPI_Instance *instance_ptr, IMMDevice *wasapi_device)
{
	assert(instance_ptr);
	assert(device_ptr);

	QUARTZ_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->device = wasapi_device;

	// pools
	quartz_poolInitialize(&device_ptr->buffers, sizeof(WASAPI_Buffer), 32);

	return QUARTZ_SUCCESS;
}
