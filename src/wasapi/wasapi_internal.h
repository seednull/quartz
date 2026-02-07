#pragma once

#include "quartz_internal.h"

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "common/pool.h"

typedef struct WASAPI_Instance_t
{
	Quartz_InstanceTable *vtbl;
	IMMDeviceEnumerator *enumerator;
} WASAPI_Instance;

typedef struct WASAPI_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceType type;
	IMMDevice *device;
	Quartz_Pool buffers;
} WASAPI_Device;

typedef struct WASAPI_Buffer_t
{
	IAudioClient *client;
	IAudioRenderClient *render_client;
	IAudioCaptureClient *capture_client;
	UINT32 size;
} WASAPI_Buffer;

Quartz_Result wasapi_helperFillDeviceInfo(IMMDevice *device, Quartz_DeviceType type, Quartz_DeviceInfo *info);
EDataFlow wasapi_helperToDataFlow(Quartz_DeviceType type);
uint32_t wasapi_helperToContainerBitDepth(Quartz_SampleFormat format);
uint32_t wasapi_helperToActualBitDepth(Quartz_SampleFormat format);
DWORD wasapi_helperToSpeakerMask(Quartz_ChannelMapping mapping);
GUID wasapi_helperToSubFormat(Quartz_SampleFormat format);
Quartz_DeviceFormat wasapi_helperToDeviceFormat(const WAVEFORMATEXTENSIBLE *format);
WAVEFORMATEXTENSIBLE wasapi_helperToWaveFormatExtensible(const Quartz_DeviceFormat *format);

Quartz_Result wasapi_fillDeviceInfo(Quartz_DeviceInfo *info);
Quartz_Result wasapi_deviceInitialize(WASAPI_Device *device_ptr, WASAPI_Instance *instance_ptr, IMMDevice *wasapi_device, Quartz_DeviceType type);
