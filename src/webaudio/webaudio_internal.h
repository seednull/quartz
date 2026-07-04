#pragma once

#include "quartz_internal.h"
#include "utils/pool.h"

#include <emscripten/webaudio.h>

#define QUARTZ_DEFINE_JS_SOURCE(name, ...) \
	static const char name[] = #__VA_ARGS__; \
	static const size_t name##_size = sizeof(name);

typedef struct WebAudio_DeviceInfo_t
{
	char id[256];
	char label[256];
} WebAudio_DeviceInfo;

typedef struct WebAudio_Instance_t
{
	Quartz_InstanceTable *vtbl;
} WebAudio_Instance;

typedef struct WebAudio_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceType type;
	WebAudio_DeviceInfo info;
	uint32_t id;
	uint32_t sample_rate;
	uint32_t channel_count;
	Quartz_Pool buffers;
} WebAudio_Device;

typedef struct WebAudio_Buffer_t
{
	uint32_t dummy;
} WebAudio_Buffer;

Quartz_Result webaudio_deviceInitialize(WebAudio_Device *device_ptr, WebAudio_Instance *instance_ptr, Quartz_DeviceType type, const WebAudio_DeviceInfo *info, uint32_t id);
