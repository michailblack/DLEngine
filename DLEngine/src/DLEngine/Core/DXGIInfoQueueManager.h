#pragma once
#include "DLEngine/Core/DLWin.h"

#include <dxgidebug.h>
#include <wrl.h>

class DXGIInfoQueueManager
{
public:
    DXGIInfoQueueManager(const DXGIInfoQueueManager&) = delete;
    DXGIInfoQueueManager(DXGIInfoQueueManager&&) = delete;
    DXGIInfoQueueManager& operator=(const DXGIInfoQueueManager&) = delete;
    DXGIInfoQueueManager& operator=(DXGIInfoQueueManager&&) = delete;

    static DXGIInfoQueueManager& Get()
    {
        static DXGIInfoQueueManager s_Instance;
        return s_Instance;
    }

    void Init();
    std::string GetMessages() const noexcept;

    bool IsValid() const noexcept { return m_DXGIInfoQueue.Get() != nullptr; }
private:
    DXGIInfoQueueManager() = default;

private:
    Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_DXGIInfoQueue;
};
