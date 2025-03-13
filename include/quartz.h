#pragma once

#include <stdint.h> // TODO: get rid of this dependency later

// Version
#define QUARTZ_VERSION_MAJOR 1
#define QUARTZ_VERSION_MINOR 0
#define QUARTZ_VERSION_PATCH 0
#define QUARTZ_VERSION "1.0.0-dev"

// Platform specific defines
#if defined(_WIN32)
	#define QUARTZ_EXPORT		__declspec(dllexport)
	#define QUARTZ_IMPORT		__declspec(dllimport)
	#define QUARTZ_INLINE		__forceinline
	#define QUARTZ_RESTRICT		__restrict
#else
	#define QUARTZ_EXPORT		__attribute__((visibility("default")))
	#define QUARTZ_IMPORT
	#define QUARTZ_INLINE		__inline__
	#define QUARTZ_RESTRICT		__restrict
#endif

#if defined(QUARTZ_SHARED_LIBRARY)
	#define QUARTZ_APIENTRY QUARTZ_EXPORT extern
#else
	#define QUARTZ_APIENTRY QUARTZ_IMPORT extern
#endif

#if !defined(QUARTZ_NULL_HANDLE)
	#define QUARTZ_NULL_HANDLE 0
#endif

#define QUARTZ_DEFINE_HANDLE(TYPE) typedef uint64_t TYPE;

#ifdef __cplusplus
extern "C" {
#endif

// Constants

// Opaque handles
QUARTZ_DEFINE_HANDLE(Quartz_Instance);
QUARTZ_DEFINE_HANDLE(Quartz_Device);

// Enums
typedef enum Quartz_Result_t
{
	QUARTZ_SUCCESS = 0,
	QUARTZ_NOT_SUPPORTED,
	QUARTZ_INVALID_OUTPUT_ARGUMENT,
	QUARTZ_INVALID_INSTANCE,
	QUARTZ_INVALID_DEVICE,
	QUARTZ_INVALID_DEVICE_INDEX,

	QUARTZ_RESULT_ENUM_MAX,
	QUARTZ_RESULT_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_Result;

typedef enum Quartz_Api_t
{
	QUARTZ_API_WASAPI = 0,
	QUARTZ_API_DIRECTSOUND,
	QUARTZ_API_NULL,

	QUARTZ_API_AUTO,

	QUARTZ_API_ENUM_MAX,
	QUARTZ_API_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_Api;

// Structs
typedef struct Quartz_InstanceDesc_t
{
	const char *application_name;
	const char *engine_name;
	uint32_t application_version;
	uint32_t engine_version;
} Quartz_InstanceDesc;

typedef struct Quartz_DeviceInfo_t
{
	char name[256];
	Quartz_Api api;
} Quartz_DeviceInfo;

// Function pointers
typedef Quartz_Result (*PFN_quartzEnumerateDevices)(Quartz_Instance instance, uint32_t *device_count, Quartz_DeviceInfo *infos);
typedef Quartz_Result (*PFN_quartzCreateDevice)(Quartz_Instance instance, uint32_t index, Quartz_Device *device);
// TODO: default device

typedef Quartz_Result (*PFN_quartzDestroyInstance)(Quartz_Instance instance);
typedef Quartz_Result (*PFN_quartzDestroyDevice)(Quartz_Device device);

typedef Quartz_Result (*PFN_quartzGetDeviceInfo)(Quartz_Device device, Quartz_DeviceInfo *info);

typedef struct Quartz_InstanceTable_t
{
	PFN_quartzEnumerateDevices enumerateDevices;
	PFN_quartzCreateDevice createDevice;
	PFN_quartzDestroyInstance destroyInstance;
} Quartz_InstanceTable;

typedef struct Quartz_DeviceTable_t
{
	PFN_quartzGetDeviceInfo getDeviceInfo;
	PFN_quartzDestroyDevice destroyDevice;
} Quartz_DeviceTable;

// API
#if !defined(QUARTZ_NO_PROTOTYPES)
QUARTZ_APIENTRY Quartz_Result quartzCreateInstance(Quartz_Api api, const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
QUARTZ_APIENTRY Quartz_Result quartzGetInstanceTable(Quartz_Instance instance, Quartz_InstanceTable *instance_table);
QUARTZ_APIENTRY Quartz_Result quartzGetDeviceTable(Quartz_Device device, Quartz_DeviceTable *device_table);

QUARTZ_APIENTRY Quartz_Result quartzEnumerateDevices(Quartz_Instance instance, uint32_t *device_count, Quartz_DeviceInfo *infos);

QUARTZ_APIENTRY Quartz_Result quartzCreateDevice(Quartz_Instance instance, uint32_t index, Quartz_Device *device);

QUARTZ_APIENTRY Quartz_Result quartzDestroyInstance(Quartz_Instance instance);
QUARTZ_APIENTRY Quartz_Result quartzDestroyDevice(Quartz_Device device);
#endif

#ifdef __cplusplus
}
#endif
