#pragma once

namespace DLEngine
{
    enum class CompareOperator
    {
        None = 0,
        Never,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Equal,
        Always,
    };
}