#include "dlpch.h"

#ifdef DL_DEBUG

#include "DXGIInfoQueue.h"

#include "DLEngine/Core/DLException.h"

#pragma comment(lib, "dxguid.lib")

namespace DLEngine
{
    void DXGIInfoQueue::Init()
    {
        using DXGIGetDebugInterface = HRESULT(WINAPI*)(REFIID, void**);

        const HMODULE dxgiDebug = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (dxgiDebug == nullptr)
        {
            DL_THROW_LAST_WIN32();
        }

        const auto dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(dxgiDebug, "DXGIGetDebugInterface")));
        if (dxgiGetDebugInterface == nullptr)
        {
            DL_THROW_LAST_WIN32();
        }

        DL_THROW_IF_HR(dxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &m_DXGIInfoQueue));

        DL_LOG_INFO_TAG("DirectX", "DXGI Info Queue Initialized");
    }

    std::string DXGIInfoQueue::GetMessages() const noexcept
    {
        std::unordered_set<std::string> messages{};

        const auto numMessages = m_DXGIInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
        for (UINT i = 0; i < numMessages; ++i)
        {
            SIZE_T messageLength = 0;
            if (SUCCEEDED(m_DXGIInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &messageLength)))
            {
                Scope<DXGI_INFO_QUEUE_MESSAGE> message{ static_cast<DXGI_INFO_QUEUE_MESSAGE*>(malloc(messageLength)) };

                if (SUCCEEDED(m_DXGIInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, message.get(), &messageLength)))
                    messages.emplace(std::string(message->pDescription) + '\n');
            }
        }
        m_DXGIInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);

        std::string messagesString{};

        for (const auto& message : messages)
            messagesString += message + '\n';

        return messagesString;
    }
}

#endif
