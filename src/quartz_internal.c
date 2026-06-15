#include "quartz_internal.h"

#include <assert.h>

#if !defined(QUARTZ_BACKEND_WASAPI)

Quartz_Result wasapi_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	QUARTZ_UNUSED(desc);
	QUARTZ_UNUSED(instance);

	return QUARTZ_NOT_SUPPORTED;
}

#endif

#if !defined(QUARTZ_BACKEND_DIRECTSOUND)

Quartz_Result directsound_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	QUARTZ_UNUSED(desc);
	QUARTZ_UNUSED(instance);

	return QUARTZ_NOT_SUPPORTED;
}

#endif
