#pragma once

#include "quartz_internal.h"

typedef struct WASAPI_Instance_t
{
	Quartz_InstanceTable *vtbl;
} WASAPI_Instance;

typedef struct WASAPI_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceInfo info;
} WASAPI_Device;

Quartz_Result wasapi_fillDeviceInfo(Quartz_DeviceInfo *info);
Quartz_Result wasapi_deviceInitialize(WASAPI_Device *device_ptr, WASAPI_Instance *instance_ptr);
