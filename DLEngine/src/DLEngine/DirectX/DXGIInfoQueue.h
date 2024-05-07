#pragma once

#ifdef DL_DEBUG

#include "DLEngine/Core/DLWin.h"
#include <dxgidebug.h>
#include <wrl.h>

namespace DLEngine
{
    class DXGIInfoQueue
    {
    public:
        DXGIInfoQueue(const DXGIInfoQueue&) = delete;
        DXGIInfoQueue(DXGIInfoQueue&&) = delete;
        DXGIInfoQueue& operator=(const DXGIInfoQueue&) = delete;
        DXGIInfoQueue& operator=(DXGIInfoQueue&&) = delete;

        static DXGIInfoQueue& Get()
        {
            static DXGIInfoQueue s_Instance;
            return s_Instance;
        }

        void Init();
        std::string GetMessages() const noexcept;
    private:
        DXGIInfoQueue() = default;

    private:
        Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_DXGIInfoQueue;
    };
}

#else

namespace DLEngine
{
    class DXGIInfoQueue
    {
    public:
        DXGIInfoQueue(const DXGIInfoQueue&) = delete;
        DXGIInfoQueue(DXGIInfoQueue&&) = delete;
        DXGIInfoQueue& operator=(const DXGIInfoQueue&) = delete;
        DXGIInfoQueue& operator=(DXGIInfoQueue&&) = delete;

        static DXGIInfoQueue& Get()
        {
            static DXGIInfoQueue s_Instance;
            return s_Instance;
        }

        void Init() {};
        std::string GetMessages() const noexcept { return ""; };
    private:
        DXGIInfoQueue() = default;
    };
}

#endif
