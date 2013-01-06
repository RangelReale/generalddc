#ifndef H__GENERALDDC_DEVICEIMPL__H
#define H__GENERALDDC_DEVICEIMPL__H

#if defined(GENERALDDC_IMPL_WIN32)
#include "GeneralDDC/DeviceImpl_WIN32.h"
#elif defined(GENERALDDC_IMPL_APPLE)
#include "GeneralDDC/DeviceImpl_APPLE.h"
#elif defined(GENERALDDC_IMPL_LINUX)
#include "GeneralDDC/DeviceImpl_LINUX.h"
#endif

#endif // H__GENERALDDC_DEVICEIMPL__H