#pragma once

#include "quartz_internal.h"

typedef struct Common_Resampler_t
{
	Quartz_ResamplerTable *vtbl;

	double ratio;
	double iratio;
	Quartz_SampleFormat src_sample_format;
	Quartz_SampleFormat dst_sample_format;
	uint32_t channel_count;
} Common_Resampler;
