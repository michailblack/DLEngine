#pragma once
#include "DLEngine/Core/DLWin.h"
#include <spdlog/spdlog.h>

namespace DLEngine
{
    class Log
    {
    public:
        static void Init();
        static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }

    private:
        inline static std::shared_ptr<spdlog::logger> s_Logger{ nullptr };
    };
}

#define DL_LOG_TRACE(...)    ::DLEngine::Log::GetLogger()->trace(__VA_ARGS__)
#define DL_LOG_DEBUG(...)    ::DLEngine::Log::GetLogger()->debug(__VA_ARGS__)
#define DL_LOG_INFO(...)     ::DLEngine::Log::GetLogger()->info(__VA_ARGS__)
#define DL_LOG_WARN(...)     ::DLEngine::Log::GetLogger()->warn(__VA_ARGS__)
#define DL_LOG_ERROR(...)    ::DLEngine::Log::GetLogger()->error(__VA_ARGS__)
#define DL_LOG_CRITICAL(...) ::DLEngine::Log::GetLogger()->critical(__VA_ARGS__)
