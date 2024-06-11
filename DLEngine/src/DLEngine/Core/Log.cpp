#include "dlpch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#pragma comment(lib, "spdlog.lib")

namespace DLEngine
{
    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

#ifdef DL_DEBUG
        spdlog::set_level(spdlog::level::trace);
#else
        spdlog::set_level(spdlog::level::info);
#endif

        s_Logger = spdlog::stdout_color_mt("DLENGINE");
    }
}
