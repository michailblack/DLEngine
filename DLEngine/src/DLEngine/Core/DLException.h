#pragma once
#include <exception>

#include "DLEngine/Core/DLWin.h"

#define DL_LAST_ERROR() DLException { static_cast<uint32_t>(__LINE__), __FILE__, GetLastError() }
#define DL_ERROR(hr) DLException { static_cast<uint32_t>(__LINE__), __FILE__, static_cast<DWORD>(hr) }
#define DL_THROW_IF(hr) if (FAILED(hr)) throw DL_ERROR(hr)

// TODO: Rewrite DXGI message capturing and add D3D11 message capturing
class DLException
    : public std::exception
{
public:
    DLException(uint32_t line, std::string file, DWORD errorCode) noexcept;

    const char* what() const noexcept override;
    virtual const char* GetType() const noexcept { return "DLEngine Exception"; }

    static std::string GetErrorString(DWORD errorCode) noexcept;

private:
    uint32_t m_Line;
    std::string m_File;
    DWORD m_ErrorCode;

    mutable std::string m_WhatBuffer;
};
