#pragma once
#include <exception>

#include "DLEngine/Core/Application.h"

#define DL_LAST_ERROR() DLException { static_cast<uint32_t>(__LINE__), __FILE__, GetLastError() }

class DLException
    : public std::exception
{
public:
    DLException(uint32_t line, const char* file, DWORD errorCode) noexcept
        : m_Line(line), m_File(file), m_ErrorCode(errorCode)
    {}

    const char* what() const noexcept override;
    virtual const char* GetType() const noexcept { return "DLEngine Exception"; }

    static std::string GetErrorString(DWORD errorCode) noexcept;

private:
    uint32_t m_Line;
    const char* m_File;
    DWORD m_ErrorCode;

    mutable std::string m_WhatBuffer;
};
