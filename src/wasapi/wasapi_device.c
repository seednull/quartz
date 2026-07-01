#include "wasapi_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

	*format = wasapi_helperToDeviceFormat(wasapi_format);

	CoTaskMemFree(wasapi_format);
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_deviceCheckFormatSupport(Quartz_Device this, const Quartz_DeviceFormat *format, uint32_t *supported)
{
	assert(this);
	assert(format);
	assert(supported);

	WASAPI_Device *device_ptr = (WASAPI_Device *)this;
	IMMDevice *wasapi_device = device_ptr->device;

	IAudioClient *wasapi_client = NULL;
	HRESULT hr = IMMDevice_Activate(wasapi_device, &IID_IAudioClient, CLSCTX_ALL, NULL, &wasapi_client);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	AUDCLNT_SHAREMODE share_mode = AUDCLNT_SHAREMODE_SHARED;

	WAVEFORMATEXTENSIBLE wasapi_format = wasapi_helperToWaveFormatExtensible(format);
	WAVEFORMATEXTENSIBLE *closest_match_format = NULL;
	hr = IAudioClient_IsFormatSupported(wasapi_client, share_mode, (const WAVEFORMATEX *)&wasapi_format, (WAVEFORMATEX **)&closest_match_format);
	IAudioClient_Release(wasapi_client);

	if (closest_match_format)
		CoTaskMemFree(closest_match_format);

	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	*supported = (hr == S_OK);
	return QUARTZ_SUCCESS;
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

	WAVEFORMATEXTENSIBLE wasapi_format = wasapi_helperToWaveFormatExtensible(&desc->format);
	WAVEFORMATEXTENSIBLE *closest_match_format = NULL;
	hr = IAudioClient_IsFormatSupported(wasapi_client, share_mode, (const WAVEFORMATEX *)&wasapi_format, (WAVEFORMATEX **)&closest_match_format);

	if (closest_match_format)
		CoTaskMemFree(closest_match_format);

	if (!SUCCEEDED(hr))
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_WASAPI_ERROR;
	}

	if (hr == S_FALSE)
	{
		IAudioClient_Release(wasapi_client);
		return QUARTZ_DEVICE_FORMAT_NOT_SUPPORTED;
	}

	REFERENCE_TIME duration = desc->duration_milliseconds * 10000;

	hr = IAudioClient_Initialize(wasapi_client, share_mode, 0, duration, 0, (const WAVEFORMATEX *)&wasapi_format, NULL);
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

	if (device_ptr->type == QUARTZ_DEVICE_TYPE_RENDER)
	{
		hr = IAudioClient_GetService(wasapi_client, &IID_IAudioRenderClient, &wasapi_render_client);
		if (!SUCCEEDED(hr))
		{
			IAudioClient_Release(wasapi_client);
			return QUARTZ_WASAPI_ERROR;
		}
	}

	if (device_ptr->type == QUARTZ_DEVICE_TYPE_CAPTURE)
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
	wasapi_deviceCheckFormatSupport,

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
