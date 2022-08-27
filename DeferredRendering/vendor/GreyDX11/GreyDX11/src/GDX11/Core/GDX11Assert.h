#pragma once

#include "Log.h"

#ifdef GDX11_DEBUG
#define GDX11_CORE_ASSERT(x, ...) { if(!(x)) { GDX11_CORE_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define GDX11_ASSERT(x, ...) { if(!(x)) { GDX11_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define GDX11_CORE_ASSERT(x, ...)
#define GDX11_ASSERT(x, ...)
#endif // GDX11_DEBUG
