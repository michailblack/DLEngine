#include "dlpch.h"
#include "DLException.h"

#include <sstream>

#include "DLEngine/Core/DXGIInfoQueueManager.h"

#pragma comment(lib, "dxgi.lib")

DLException::DLException(uint32_t line, std::string file, DWORD errorCode) noexcept
    : m_Line(line), m_File(std::move(file)), m_ErrorCode(errorCode)
{
}

const char* DLException::what() const noexcept
{
    std::ostringstream oss;
    oss << "[Error Code] 0x" << std::hex << std::uppercase << m_ErrorCode
		<< std::dec << " (" << static_cast<uint32_t>(m_ErrorCode) << ")" << '\n'
        << "[Description] " << GetErrorString(m_ErrorCode) << '\n'
        << "[File] " << m_File << '\n'
        << "[Line] " << m_Line << '\n';

    m_WhatBuffer = oss.str();
    return m_WhatBuffer.c_str();
}

std::string DLException::GetErrorString(DWORD errorCode) noexcept
{
    char* msgBuf { nullptr };
    const DWORD msgLen = FormatMessageA(
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
        return "Unidentified error code";
    }

    std::string errorString { msgBuf, msgBuf + msgLen };
    LocalFree(msgBuf);

#ifdef DL_DEBUG
    if (!DXGIInfoQueueManager::Get().IsValid())
        return errorString;

    const auto& d3dErrorStr { DXGIInfoQueueManager::Get().GetMessages() };

    if (d3dErrorStr.empty())
        return errorString;

    errorString.append("\n[Additional DXGI Info]\n");

    errorString.append(d3dErrorStr.begin(), d3dErrorStr.end());
#endif
    return errorString;
}
