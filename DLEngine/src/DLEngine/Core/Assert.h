#pragma once
#include "DLEngine/Core/Log.h"

#ifdef DL_ENABLE_ASSERTS
#define DL_ASSERT(condition, ...) { if(!(condition)) { ::DLEngine::Log::PrintAssertMessage("Assertion Failed" __VA_OPT__(,) __VA_ARGS__); __debugbreak(); } }

#ifdef DL_DEBUG
#define DL_VERIFY(expr, ...) { if(!(expr)) { ::DLEngine::Log::PrintAssertMessage("Verification Failed" __VA_OPT__(,) __VA_ARGS__); __debugbreak(); } }
#else
#define DL_VERIFY(expr, ...) (expr)
#endif

#else
#define DL_ASSERT(x, ...)
#define DL_VERIFY(expr, ...) (expr)
#endif