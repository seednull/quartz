#pragma once

#include "quartz_internal.h"

typedef struct Null_Instance_t
{
	Quartz_InstanceTable *vtbl;
	char *application_name;
	uint32_t application_version;
	char *engine_name;
	uint32_t engine_version;
} Null_Instance;

typedef struct Null_Device_t
{
	Quartz_DeviceTable *vtbl;
	Quartz_DeviceInfo info;
} Null_Device;

Quartz_Result null_fillDeviceInfo(Quartz_DeviceType type, Quartz_DeviceInfo *info);
Quartz_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr, Quartz_DeviceType type);
