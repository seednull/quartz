#include "common_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>

static QUARTZ_INLINE double quartz_clamp(double value, double min, double max)
{
	return (value > max) ? max : (value < min) ? min : value;
}

static QUARTZ_INLINE uint32_t quartz_min(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}

static QUARTZ_INLINE float quartz_lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

static QUARTZ_INLINE float quartz_readSample(const void *frames, uint32_t frame, uint32_t channel_count, uint32_t channel, Quartz_SampleFormat format)
{
	assert(frames);
	assert(channel_count > 0);
	assert(channel < channel_count);
	assert(format != QUARTZ_SAMPLE_FORMAT_UNKNOWN);

	uint32_t index = frame * channel_count + channel;

	switch (format)
	{
		case QUARTZ_SAMPLE_FORMAT_UINT8:
		{
			const uint8_t *ptr = (const uint8_t *)frames;
			return (float)((double)(ptr[index] - 127.5) / 127.5);
		}
		case QUARTZ_SAMPLE_FORMAT_SINT16:
		{
			const int16_t *ptr = (const int16_t *)frames;
			return (float)((double)ptr[index] / 32768.0);
		}
		case QUARTZ_SAMPLE_FORMAT_SINT24:
		{
			const int32_t *ptr = (const int32_t *)frames;
			int32_t value = ptr[index];

			if (value & 0x00800000)
				value |= 0xFF000000;

			return (float)((double)value / 8388608.0);
		}
		case QUARTZ_SAMPLE_FORMAT_SINT32:
		{
			const int32_t *ptr = (const int32_t *)frames;
			return (float)((double)ptr[index] / 2147483648.0);
		}
		case QUARTZ_SAMPLE_FORMAT_FLOAT32:
		{
			const float *ptr = (const float *)frames;
			return ptr[index];
		}
		default: return 0.0f;
	}
}

static QUARTZ_INLINE void quartz_writeSample(void *frames, uint32_t frame, uint32_t channel_count, uint32_t channel, Quartz_SampleFormat format, float value)
{
	assert(frames);
	assert(channel_count > 0);
	assert(channel < channel_count);
	assert(format != QUARTZ_SAMPLE_FORMAT_UNKNOWN);

	uint32_t index = frame * channel_count + channel;

	switch (format)
	{
		case QUARTZ_SAMPLE_FORMAT_UINT8:
		{
			uint8_t *ptr = (uint8_t *)frames;
			ptr[index] = (uint8_t)(quartz_clamp((double)value * 127.5 + 127.5, 0.0, 255.0));
		}
		break;
		case QUARTZ_SAMPLE_FORMAT_SINT16:
		{
			int16_t *ptr = (int16_t *)frames;
			ptr[index] = (int16_t)(quartz_clamp((double)value * 32768.0, -32768.0, 32767.0));
		}
		break;
		case QUARTZ_SAMPLE_FORMAT_SINT24:
		{
			int32_t *ptr = (int32_t *)frames;
			ptr[index] = (int32_t)(quartz_clamp((double)value * 8388608.0, -8388608.0, 8388607.0));
		}
		break;
		case QUARTZ_SAMPLE_FORMAT_SINT32:
		{
			int32_t *ptr = (int32_t *)frames;
			ptr[index] = (int32_t)(quartz_clamp((double)value * 2147483648.0, -2147483648.0, 2147483647.0));
		}
		break;
		case QUARTZ_SAMPLE_FORMAT_FLOAT32:
		{
			float *ptr = (float *)frames;
			ptr[index] = value;
		}
		break;
		default: return;
	}
}

/*
 */
static Quartz_Result common_resamplerLinearResampleFrames(Quartz_Resampler this, const void *src_frames, uint32_t src_frame_count, void *dst_frames, uint32_t dst_frame_count, uint32_t *frames_written)
{
	assert(this);
	assert(src_frames);
	assert(src_frame_count > 0);
	assert(dst_frames);
	assert(dst_frame_count > 0);
	assert(frames_written);

	Common_Resampler *ptr = (Common_Resampler *)this;
	uint32_t channel_count = ptr->channel_count;

	for (uint32_t i = 0; i < dst_frame_count; ++i)
	{
		double position = i * ptr->ratio;
		uint32_t src_frame0 = (uint32_t)position;
		uint32_t src_frame1 = quartz_min(src_frame0 + 1, src_frame_count - 1);

		assert(src_frame0 < src_frame_count);
		assert(src_frame1 < src_frame_count);

		if (src_frame0 == src_frame1)
		{
			for (uint32_t j = 0; j < channel_count; ++j)
			{
				float src_sample = quartz_readSample(src_frames, src_frame0, channel_count, j, ptr->src_sample_format);
				quartz_writeSample(dst_frames, i, channel_count, j, ptr->dst_sample_format, src_sample);
			}
		}
		else
		{
			float frac = (float)(position - src_frame0);
			for (uint32_t j = 0; j < channel_count; ++j)
			{
				float src_sample0 = quartz_readSample(src_frames, src_frame0, channel_count, j, ptr->src_sample_format);
				float src_sample1 = quartz_readSample(src_frames, src_frame1, channel_count, j, ptr->src_sample_format);

				float dst_sample = quartz_lerp(src_sample0, src_sample1, frac);
				quartz_writeSample(dst_frames, i, channel_count, j, ptr->dst_sample_format, dst_sample);
			}
		}
	}

	*frames_written = dst_frame_count;
	return QUARTZ_SUCCESS;
}

static Quartz_Result common_resamplerLinearFlushRemainingFrames(Quartz_Resampler this, void *dst_frames, uint32_t dst_frame_count, uint32_t *frames_written)
{
	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(dst_frames);
	QUARTZ_UNUSED(dst_frame_count);
	assert(frames_written);

	*frames_written = 0;
	return QUARTZ_SUCCESS;
}

static Quartz_Result common_resamplerLinearCalculateSourceFrameCount(Quartz_Resampler this, uint32_t dst_frame_count, uint32_t *src_frame_count)
{
	assert(this);
	assert(dst_frame_count > 0);
	assert(src_frame_count);

	Common_Resampler *ptr = (Common_Resampler *)this;

	*src_frame_count = (uint32_t)round(dst_frame_count * ptr->ratio);
	return QUARTZ_SUCCESS;
}

static Quartz_Result common_resamplerLinearCalculateDestinationFrameCount(Quartz_Resampler this, uint32_t src_frame_count, uint32_t *dst_frame_count)
{
	assert(this);
	assert(src_frame_count > 0);
	assert(dst_frame_count);

	Common_Resampler *ptr = (Common_Resampler *)this;

	*dst_frame_count = (uint32_t)round(src_frame_count * ptr->iratio);
	return QUARTZ_SUCCESS;
}

static Quartz_Result common_resamplerDestroy(Quartz_Resampler this)
{
	assert(this);

	Common_Resampler *ptr = (Common_Resampler *)this;

	free(ptr);

	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_ResamplerTable resampler_linear_vtbl =
{
	common_resamplerLinearResampleFrames,
	common_resamplerLinearFlushRemainingFrames,
	common_resamplerLinearCalculateSourceFrameCount,
	common_resamplerLinearCalculateDestinationFrameCount,

	common_resamplerDestroy,
};

/*
 */
Quartz_Result common_createResampler(const Quartz_ResamplerDesc *desc, Quartz_Resampler *resampler)
{
	assert(desc);
	assert(desc->src_sample_rate > 0);
	assert(desc->dst_sample_rate > 0);
	assert(desc->src_sample_format != QUARTZ_SAMPLE_FORMAT_UNKNOWN);
	assert(desc->dst_sample_format != QUARTZ_SAMPLE_FORMAT_UNKNOWN);
	assert(desc->channel_count > 0);
	assert(resampler);

	Common_Resampler *ptr = (Common_Resampler *)malloc(sizeof(Common_Resampler));
	assert(ptr);

	// vtable
	ptr->vtbl = &resampler_linear_vtbl;

	// data
	ptr->ratio = (double)desc->src_sample_rate / desc->dst_sample_rate;
	ptr->iratio = 1.0 / ptr->ratio;
	ptr->src_sample_format = desc->src_sample_format;
	ptr->dst_sample_format = desc->dst_sample_format;
	ptr->channel_count = desc->channel_count;

	*resampler = (Quartz_Resampler)ptr;
	return QUARTZ_SUCCESS;
}
