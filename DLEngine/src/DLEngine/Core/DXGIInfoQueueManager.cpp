#include "dlpch.h"
#include "DXGIInfoQueueManager.h"

#pragma comment(lib, "dxguid.lib")

void DXGIInfoQueueManager::Init()
{
#ifdef DL_DEBUG
    using DXGIGetDebugInterface = HRESULT(WINAPI*)(REFIID, void**);

    const HMODULE dxgiDebug = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (dxgiDebug == nullptr)
    {
        throw DL_LAST_ERROR();
    }

    const auto dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(dxgiDebug, "DXGIGetDebugInterface")));
    if (dxgiGetDebugInterface == nullptr)
    {
        throw DL_LAST_ERROR();
    }

    Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
    DL_THROW_IF(dxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &dxgiInfoQueue));

    DL_THROW_IF(dxgiInfoQueue.As(&m_DXGIInfoQueue));
#endif
}

std::string DXGIInfoQueueManager::GetMessages() const noexcept
{
    std::string messages {};

#ifdef DL_DEBUG
    const auto numMessages = m_DXGIInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    for (UINT i = 0; i < numMessages; ++i)
    {
        SIZE_T messageLength = 0;
        m_DXGIInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &messageLength);

        Scope<DXGI_INFO_QUEUE_MESSAGE> message { static_cast<DXGI_INFO_QUEUE_MESSAGE*>(malloc(messageLength)) };

        m_DXGIInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, message.get(), &messageLength);

        messages += " - " + std::string(message->pDescription) + "\n";
    }
    m_DXGIInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
#endif

    return messages;
}
