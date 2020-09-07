#ifndef KVM_PLATFORM_TYPES_H
#define KVM_PLATFORM_TYPES_H

#if defined(KVM_OS_WINDOWS)
#include <platform/windows/types.h>
#elif defined(KVM_OS_LINUX)
#include <platform/linux/types.h>
#include <platform/unix/types.h>
#elif defined(KVM_OS_MAC)
#include <platform/mac/types.h>
#include <platform/unix/types.h>
#endif

#endif // KVM_PLATFORM_TYPES_H