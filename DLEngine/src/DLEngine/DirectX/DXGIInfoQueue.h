#pragma once

#ifdef DL_DEBUG

#include "DLEngine/Core/DLWin.h"
#include <dxgidebug.h>
#include <wrl.h>

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

    bool IsValid() const noexcept { return m_DXGIInfoQueue.Get() != nullptr; }
private:
    DXGIInfoQueue() = default;

private:
    Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_DXGIInfoQueue;
};
#endif
