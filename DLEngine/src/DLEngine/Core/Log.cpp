#include "dlpch.h"
#include "Log.h"

#ifdef DL_DEBUG

#include <spdlog/sinks/stdout_color_sinks.h>

#pragma comment(lib, "spdlog.lib")

namespace DLEngine
{
    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        spdlog::set_level(spdlog::level::trace);

        s_Logger = spdlog::stdout_color_mt("DLENGINE");
    }

    void Log::SetLogLevel(spdlog::level::level_enum level)
    {
        spdlog::set_level(level);
    }
}

#endif // DL_DEBUG
