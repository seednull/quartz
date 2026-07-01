#include "quartz_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
typedef struct Quartz_ResamplerInternal_t
{
	Quartz_ResamplerTable *vtbl;
} Quartz_ResamplerInternal;

typedef struct Quartz_InstanceInternal_t
{
	Quartz_InstanceTable *vtbl;
} Quartz_InstanceInternal;

typedef struct Quartz_DeviceInternal_t
{
	Quartz_DeviceTable *vtbl;
} Quartz_DeviceInternal;

/*
 */
Quartz_Result quartzCreateResampler(const Quartz_ResamplerDesc *desc, Quartz_Resampler *resampler)
{
	return common_quartzCreateResampler(desc, resampler);
}

Quartz_Result quartzGetResamplerTable(Quartz_Resampler resampler, Quartz_ResamplerTable *resampler_table)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	if (resampler_table == NULL)
		return QUARTZ_INVALID_OUTPUT_ARGUMENT;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);

	memcpy(resampler_table, ptr->vtbl, sizeof(Quartz_ResamplerTable));
	return QUARTZ_SUCCESS;
}

Quartz_Result quartzResampleFrames(Quartz_Resampler resampler, const void *src_frames, uint64_t src_frame_count, void *dst_frames, uint64_t dst_frame_count, uint64_t *frames_written)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);
	assert(ptr->vtbl->resampleFrames);

	return ptr->vtbl->resampleFrames(resampler, src_frames, src_frame_count, dst_frames, dst_frame_count, frames_written);
}

Quartz_Result quartzFlushRemainingFrames(Quartz_Resampler resampler, void *dst_frames, uint64_t dst_frame_count, uint64_t *frames_written)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);
	assert(ptr->vtbl->flushRemainingFrames);

	return ptr->vtbl->flushRemainingFrames(resampler, dst_frames, dst_frame_count, frames_written);
}

Quartz_Result quartzCalculateSourceFrameCount(Quartz_Resampler resampler, uint64_t dst_frame_count, uint64_t *src_frame_count)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);
	assert(ptr->vtbl->calculateSourceFrameCount);

	return ptr->vtbl->calculateSourceFrameCount(resampler, dst_frame_count, src_frame_count);
}

Quartz_Result quartzCalculateDestinationFrameCount(Quartz_Resampler resampler, uint64_t src_frame_count, uint64_t *dst_frame_count)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);
	assert(ptr->vtbl->calculateDestinationFrameCount);

	return ptr->vtbl->calculateDestinationFrameCount(resampler, src_frame_count, dst_frame_count);
}

Quartz_Result quartzDestroyResampler(Quartz_Resampler resampler)
{
	if (resampler == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_RESAMPLER;

	Quartz_ResamplerInternal *ptr = (Quartz_ResamplerInternal *)resampler;
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyResampler);

	return ptr->vtbl->destroyResampler(resampler);
}

/*
 */
Quartz_Result quartzCreateInstance(Quartz_Api api, const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	switch (api)
	{
		case QUARTZ_API_WASAPI: return wasapi_quartzCreateInstance(desc, instance);
		case QUARTZ_API_DIRECTSOUND: return directsound_quartzCreateInstance(desc, instance);
		case QUARTZ_API_WEBAUDIO: return webaudio_quartzCreateInstance(desc, instance);
		case QUARTZ_API_NULL: return null_quartzCreateInstance(desc, instance);

		case QUARTZ_API_AUTO:
		{
			Quartz_Result result = QUARTZ_NOT_SUPPORTED;

#if QUARTZ_BACKEND_WASAPI
			if (result != QUARTZ_SUCCESS)
				result = wasapi_quartzCreateInstance(desc, instance);
#endif

#if QUARTZ_BACKEND_DIRECTSOUND
			if (result != QUARTZ_SUCCESS)
				result = directsound_quartzCreateInstance(desc, instance);
#endif

#if QUARTZ_BACKEND_WEBAUDIO
			if (result != QUARTZ_SUCCESS)
				result = webaudio_quartzCreateInstance(desc, instance);
#endif

			return result;
		}

		default: return QUARTZ_NOT_SUPPORTED;
	}
}

Quartz_Result quartzGetInstanceTable(Quartz_Instance instance, Quartz_InstanceTable *instance_table)
{
	if (instance == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_INSTANCE;

	if (instance_table == NULL)
		return QUARTZ_INVALID_OUTPUT_ARGUMENT;

	Quartz_InstanceInternal *ptr = (Quartz_InstanceInternal *)instance;
	assert(ptr->vtbl);

	memcpy(instance_table, ptr->vtbl, sizeof(Quartz_InstanceTable));
	return QUARTZ_SUCCESS;
}

Quartz_Result quartzGetDeviceTable(Quartz_Device device, Quartz_DeviceTable *device_table)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	if (device_table == NULL)
		return QUARTZ_INVALID_OUTPUT_ARGUMENT;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)device;
	assert(ptr->vtbl);

	memcpy(device_table, ptr->vtbl, sizeof(Quartz_DeviceTable));
	return QUARTZ_SUCCESS;
}

/*
 */
Quartz_Result quartzEnumerateDevices(Quartz_Instance instance, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	if (instance == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_INSTANCE;

	Quartz_InstanceInternal *ptr = (Quartz_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->enumerateDevices);

	return ptr->vtbl->enumerateDevices(instance, type, device_count, infos);
}

Quartz_Result quartzCreateDevice(Quartz_Instance instance, Quartz_DeviceType type, uint32_t index, Quartz_Device *device)
{
	if (instance == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_INSTANCE;

	Quartz_InstanceInternal *ptr = (Quartz_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->createDevice);

	return ptr->vtbl->createDevice(instance, type, index, device);
}

Quartz_Result quartzCreateDefaultDevice(Quartz_Instance instance, Quartz_DeviceType type, Quartz_Device *device)
{
	if (instance == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_INSTANCE;

	Quartz_InstanceInternal *ptr = (Quartz_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->createDefaultDevice);

	return ptr->vtbl->createDefaultDevice(instance, type, device);
}

/*
 */
Quartz_Result quartzDestroyInstance(Quartz_Instance instance)
{
	if (instance == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_INSTANCE;

	Quartz_InstanceInternal *ptr = (Quartz_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyInstance);

	return ptr->vtbl->destroyInstance(instance);
}

/*
 */
Quartz_Result quartzGetDeviceInfo(Quartz_Device device, Quartz_DeviceInfo *info)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getDeviceInfo);

	return ptr->vtbl->getDeviceInfo(device, info);
}

Quartz_Result quartzGetPreferredFormat(Quartz_Device device, Quartz_DeviceFormat *format)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getPreferredFormat);

	return ptr->vtbl->getPreferredFormat(device, format);
}

Quartz_Result quartzCheckFormatSupport(Quartz_Device device, const Quartz_DeviceFormat *format, uint32_t *supported)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->checkFormatSupport);

	return ptr->vtbl->checkFormatSupport(device, format, supported);
}

/*
 */
Quartz_Result quartzCreateBuffer(Quartz_Device device, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createBuffer);

	return ptr->vtbl->createBuffer(device, desc, buffer);
}

/*
 */
Quartz_Result quartzDestroyBuffer(Quartz_Device device, Quartz_Buffer buffer)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyBuffer);

	return ptr->vtbl->destroyBuffer(device, buffer);
}

Quartz_Result quartzDestroyDevice(Quartz_Device device)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyDevice);

	return ptr->vtbl->destroyDevice(device);
}

/*
 */
Quartz_Result quartzStart(Quartz_Device device, Quartz_Buffer buffer)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->start);

	return ptr->vtbl->start(device, buffer);
}

Quartz_Result quartzStop(Quartz_Device device, Quartz_Buffer buffer)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->stop);

	return ptr->vtbl->stop(device, buffer);
}


Quartz_Result quartzBeginRender(Quartz_Device device, Quartz_Buffer buffer, void **mapped_ptr, uint32_t *frame_count)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->beginRender);

	return ptr->vtbl->beginRender(device, buffer, mapped_ptr, frame_count);
}

Quartz_Result quartzEndRender(Quartz_Device device, Quartz_Buffer buffer, uint32_t frames_written)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->endRender);

	return ptr->vtbl->endRender(device, buffer, frames_written);
}


Quartz_Result quartzBeginCapture(Quartz_Device device, Quartz_Buffer buffer, void **mapped_ptr, uint32_t *frame_count)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->beginCapture);

	return ptr->vtbl->beginCapture(device, buffer, mapped_ptr, frame_count);
}

Quartz_Result quartzEndCapture(Quartz_Device device, Quartz_Buffer buffer, uint32_t frames_read)
{
	if (device == QUARTZ_NULL_HANDLE)
		return QUARTZ_INVALID_DEVICE;

	Quartz_DeviceInternal *ptr = (Quartz_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->endCapture);

	return ptr->vtbl->endCapture(device, buffer, frames_read);
}
