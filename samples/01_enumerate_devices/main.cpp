#include <quartz.h>
#include <cassert>
#include <iostream>

void printDeviceInfo(const Quartz_DeviceInfo *info)
{
	std::cout << "Device name: " << info->name << "\n";
	std::cout << "\n";
}

void testEnumerateDevices(Quartz_Instance instance)
{
	uint32_t device_count = 0;
	Quartz_Result result = quartzEnumerateDevices(instance, &device_count, nullptr);
	if (result != QUARTZ_SUCCESS)
		return;

	Quartz_DeviceInfo *infos = (Quartz_DeviceInfo *)malloc(sizeof(Quartz_DeviceInfo) * device_count);
	result = quartzEnumerateDevices(instance, &device_count, infos);
	assert(result == QUARTZ_SUCCESS);

	for (uint32_t i = 0; i < device_count; ++i)
	{
		std::cout << " ------ [Enumerate " << i << "] ------\n";
		printDeviceInfo(&infos[i]);
	}

	free(infos);
}

void testCreateDevice(Quartz_Instance instance, uint32_t index)
{
	Quartz_Device device = QUARTZ_NULL_HANDLE;
	Quartz_Result result = quartzCreateDevice(instance, index, &device);
	if (result != QUARTZ_SUCCESS)
		return;

	Quartz_DeviceInfo info = {};
	result = quartzGetDeviceInfo(device, &info);
	assert(result == QUARTZ_SUCCESS);
	std::cout << " ------ [CreateByIndex " << index << "] ------\n";
	printDeviceInfo(&info);

	result = quartzDestroyDevice(device);
	assert(result == QUARTZ_SUCCESS);
}

int main()
{
	Quartz_Instance instance = QUARTZ_NULL_HANDLE;

	Quartz_InstanceDesc instance_desc =
	{
		"01_enumerate_devices",
		"Quartz",
		0,
		0,
	};

	Quartz_Result result = quartzCreateInstance(QUARTZ_API_WASAPI, &instance_desc, &instance);
	assert(result == QUARTZ_SUCCESS);

	testEnumerateDevices(instance);
	testCreateDevice(instance, 0);

	result = quartzDestroyInstance(instance);
	assert(result == QUARTZ_SUCCESS);

	return 0;
}
