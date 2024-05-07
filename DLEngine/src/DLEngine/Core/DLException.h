﻿#pragma once
#include "DLEngine/Core/DLWin.h"
#include <exception>

#define DL_THROW_LAST_WIN32() throw DLEngine::HRException{ static_cast<uint32_t>(__LINE__), __FILE__, HRESULT_FROM_WIN32(GetLastError()) }
#define DL_THROW_IF_HR(hrCall) { const auto hr{ (hrCall) }; if (FAILED(hr)) throw DLEngine::HRException { static_cast<uint32_t>(__LINE__), __FILE__, hr }; }

#ifdef DL_DEBUG

#define DL_THROW_IF_D3D11(d3d11DrawCall)\
    {\
        const uint64_t numMsg { DLEngine::D3D::GetInfoQueue()->GetNumStoredMessages() };\
        (d3d11DrawCall);\
        if (numMsg != DLEngine::D3D::GetInfoQueue()->GetNumStoredMessages())\
        {\
            throw DLEngine::D3D11Exception { static_cast<uint32_t>(__LINE__), __FILE__ };\
        }\
        DLEngine::D3D::GetInfoQueue()->ClearStoredMessages();\
    }

#else

#define DL_THROW_IF_D3D11(d3d11DrawCall) (d3d11DrawCall);

#endif // DL_DEBUG

namespace DLEngine
{
    class DLException
        : public std::exception
    {
    public:
        DLException(uint32_t line, std::string file) noexcept;

        const char* what() const noexcept override;
        virtual const char* GetType() const noexcept { return "DLEngine Exception"; }

    protected:
        uint32_t m_Line;
        std::string m_File;

        mutable std::string m_WhatBuffer;
    };

    class HRException
        : public DLException
    {
    public:
        HRException(uint32_t line, std::string file, HRESULT errorCode) noexcept;

        const char* what() const noexcept override;
        const char* GetType() const noexcept override { return "HRESULT Exception"; }

    private:
        HRESULT m_ErrorCode;
    };

#ifdef DL_DEBUG

    class D3D11Exception
        : public DLException
    {
    public:
        D3D11Exception(uint32_t line, std::string file) noexcept;

        const char* what() const noexcept override;
        const char* GetType() const noexcept override { return "D3D11 Exception"; }
    };

#endif // DL_DEBUG
}
