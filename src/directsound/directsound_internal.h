#pragma once

#include "quartz_internal.h"

typedef struct DirectSound_Instance_t
{
	Quartz_InstanceTable *vtbl;
} DirectSound_Instance;

typedef struct DirectSound_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceInfo info;
} DirectSound_Device;

Quartz_Result directsound_fillDeviceInfo(Quartz_DeviceInfo *info);
Quartz_Result directsound_deviceInitialize(DirectSound_Device *device_ptr, DirectSound_Instance *instance_ptr);
