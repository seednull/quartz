#include <quartz.h>

#include <vector>
#include <cassert>

int main()
{
	Quartz_Resampler resampler = QUARTZ_NULL_HANDLE;

	Quartz_ResamplerDesc desc =
	{
		QUARTZ_SAMPLE_RATE_44100,
		QUARTZ_SAMPLE_RATE_48000,
		QUARTZ_SAMPLE_FORMAT_FLOAT32,
		QUARTZ_SAMPLE_FORMAT_FLOAT32,
		2
	};

	Quartz_Result result = quartzCreateResampler(&desc, &resampler);
	assert(result == QUARTZ_SUCCESS);

	std::vector<float> src_data(QUARTZ_SAMPLE_RATE_192000 * desc.channel_count);
	std::vector<float> dst_data(QUARTZ_SAMPLE_RATE_192000 * desc.channel_count);

	for (uint32_t i = 1; i < QUARTZ_SAMPLE_RATE_192000; ++i)
	{
		uint32_t dst_frames = 0;
		result = quartzCalculateDestinationFrameCount(resampler, i, &dst_frames);
		assert(result == QUARTZ_SUCCESS);

		uint32_t src_frames = 0;
		result = quartzCalculateSourceFrameCount(resampler, dst_frames, &src_frames);
		assert(result == QUARTZ_SUCCESS);
		assert(src_frames == i);

		src_data.resize(src_frames * desc.channel_count);
		dst_data.resize(dst_frames * desc.channel_count);

		uint32_t frames_written = 0;
		result = quartzResampleFrames(resampler, src_data.data(), src_frames, dst_data.data(), dst_frames, &frames_written);
		assert(result == QUARTZ_SUCCESS);
		assert(frames_written == dst_frames);
	}

	result = quartzDestroyResampler(resampler);
	assert(result == QUARTZ_SUCCESS);

	return 0;
}
