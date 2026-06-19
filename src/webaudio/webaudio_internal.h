#pragma once

#include "quartz_internal.h"
#include "utils/pool.h"

#include <emscripten/webaudio.h>

typedef struct WebAudio_Instance_t
{
	Quartz_InstanceTable *vtbl;
} WebAudio_Instance;

typedef struct WebAudio_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceType type;
	Quartz_Pool buffers;
} WebAudio_Device;

typedef struct WebAudio_Buffer_t
{
} WebAudio_Buffer;

Quartz_Result webaudio_deviceInitialize(WebAudio_Device *device_ptr, WebAudio_Instance *instance_ptr, Quartz_DeviceType type);
