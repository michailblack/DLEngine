#include "dlpch.h"
#include "DLException.h"

#include <d3d11sdklayers.h>
#include <sstream>

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/DXGIInfoQueue.h"

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            std::string GetErrorString(DWORD errorCode) noexcept
            {
                char* msgBuf{ nullptr };
                DWORD msgLen = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    errorCode,
                    0,
                    reinterpret_cast<LPSTR>(&msgBuf),
                    0,
                    nullptr
                );

                if (msgLen == 0)
                {
                    // The error code did not exist in the system errors.
                    // Try Ntdsbmsg.dll for the error code.

                    const HINSTANCE hInst = LoadLibraryA("Ntdsbmsg.dll");
                    if (!hInst)
                    {
                        LocalFree(msgBuf);
                        return "Failed to get the description\n";
                    }

                    // Try getting message text from ntdsbmsg.
                    msgLen = FormatMessageA(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                        hInst,
                        errorCode,
                        0,
                        reinterpret_cast<LPSTR>(&msgBuf),
                        0,
                        nullptr
                    );

                    FreeLibrary(hInst);

                    if (msgLen == 0)
                    {
                        LocalFree(msgBuf);
                        return "Failed to get the description\n";
                    }
                }

                const std::string errorString{ msgBuf };
                LocalFree(msgBuf);

                return errorString + '\n';
            }
        }
    }

    DLException::DLException(uint32_t line, std::string file) noexcept
        : m_Line(line), m_File(std::move(file))
    {
    }

    const char* DLException::what() const noexcept
    {
        std::ostringstream oss;
        oss << "[Error] Unidentified error\n"
            << "[Description] No description provided\n"
            << "[File] " << m_File << '\n'
            << "[Line] " << m_Line << '\n';

        m_WhatBuffer = oss.str();
        return m_WhatBuffer.c_str();
    }

    HRException::HRException(uint32_t line, std::string file, HRESULT errorCode) noexcept
        : DLException(line, std::move(file))
        , m_ErrorCode(errorCode)
    {
    }

    const char* HRException::what() const noexcept
    {
        std::string description{ Utils::GetErrorString(m_ErrorCode) };

        const std::string dxgiErrors{ DXGIInfoQueue::Get().GetMessages() };
        if (!dxgiErrors.empty())
            description += "[DXGI INFO]\n" + dxgiErrors;

        std::ostringstream oss;
        oss << "[Error] 0x" << std::hex << std::uppercase << m_ErrorCode
            << std::dec << ' ' << static_cast<uint64_t>(m_ErrorCode) << ' ' << '\n' << '\n'
            << "[Description] " << description
            << "[File] " << m_File << '\n'
            << "[Line] " << m_Line << '\n';

        m_WhatBuffer = oss.str();
        return m_WhatBuffer.c_str();
    }

#ifdef DL_DEBUG

    D3D11Exception::D3D11Exception(uint32_t line, std::string file) noexcept
        : DLException(line, std::move(file))
    {
    }

    const char* D3D11Exception::what() const noexcept
    {
        using namespace Microsoft::WRL;

        const auto& infoQueue = D3D::GetInfoQueue();
        infoQueue->PushEmptyRetrievalFilter();
        const uint64_t messageCount{ infoQueue->GetNumStoredMessages() };
        std::unordered_set<std::string> messages{};

        for (uint64_t i = 0; i < messageCount; ++i)
        {
            size_t messageLength{ 0 };
            if (SUCCEEDED(infoQueue->GetMessageW(i, nullptr, &messageLength)))
            {
                std::vector<char> buffer(messageLength);
                D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(buffer.data());
                if (SUCCEEDED(infoQueue->GetMessageW(i, message, &messageLength)))
                {
                    messages.emplace(std::string(message->pDescription) + '\n');
                }
            }
        }

        std::string description{};

        for (const std::string& message : messages)
            description += message + '\n';

        if (description.empty())
            description = "Failed to get the description\n";

        std::ostringstream oss;
        oss << "[Error] D3D11 Error(s)" << '\n' << '\n'
            << "[Description]\n" << description
            << "[File] " << m_File << '\n'
            << "[Line] " << m_Line << '\n';

        m_WhatBuffer = oss.str();
        return m_WhatBuffer.c_str();
    }

#endif // DL_DEBUG
}
