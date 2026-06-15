#pragma once

#include <quartz.h>

#define QUARTZ_UNUSED(x) do { (void)(x); } while(0)

Quartz_Result common_quartzCreateResampler(const Quartz_ResamplerDesc *desc, Quartz_Resampler *resampler);
Quartz_Result wasapi_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
Quartz_Result directsound_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
Quartz_Result null_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
