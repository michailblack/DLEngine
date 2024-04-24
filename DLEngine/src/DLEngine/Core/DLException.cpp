#include "dlpch.h"
#include "DLException.h"

#include <sstream>

const char* DLException::what() const noexcept
{
    std::ostringstream oss;
    oss << "[Error Code] 0x" << std::hex << std::uppercase << m_ErrorCode
		<< std::dec << " (" << static_cast<uint32_t>(m_ErrorCode) << ")" << std::endl
        << "[Description] " << GetErrorString(m_ErrorCode) << std::endl
        << "[Line] " << m_Line << std::endl
        << "[File] " << m_File << std::endl;

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

    return errorString;
}
