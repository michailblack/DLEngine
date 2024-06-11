#include "dlpch.h"
#include "Utils.h"

namespace DLEngine::Utils
{
    std::string WideStrToMultiByteStr(const std::wstring& wideStr)
    {
        size_t wideStrSize{ wideStr.size() + 1u };
        std::string multiByteStr{};
        multiByteStr.resize(wideStrSize);

        wcstombs_s(nullptr, multiByteStr.data(), wideStrSize, wideStr.data(), wideStr.size());

        return multiByteStr;
    }
}
