#pragma once

#include <quartz.h>

#define QUARTZ_UNUSED(x) do { (void)(x); } while(0)

Quartz_Result wasapi_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
Quartz_Result directsound_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
Quartz_Result null_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
