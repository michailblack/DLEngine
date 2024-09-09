#pragma once
#include "DLEngine/Core/Assert.h"

#include "DLEngine/Renderer/Common.h"

#include <d3d11_4.h>

namespace DLEngine
{
    namespace Utils
    {
        inline D3D11_COMPARISON_FUNC D3D11ComparisonFuncFromCompareOp(CompareOperator compareOp)
        {
            switch (compareOp)
            {
            case CompareOperator::Never:          return D3D11_COMPARISON_NEVER;
            case CompareOperator::NotEqual:       return D3D11_COMPARISON_NOT_EQUAL;
            case CompareOperator::Less:           return D3D11_COMPARISON_LESS;
            case CompareOperator::LessOrEqual:    return D3D11_COMPARISON_LESS_EQUAL;
            case CompareOperator::Greater:        return D3D11_COMPARISON_GREATER;
            case CompareOperator::GreaterOrEqual: return D3D11_COMPARISON_GREATER_EQUAL;
            case CompareOperator::Equal:          return D3D11_COMPARISON_EQUAL;
            case CompareOperator::Always:         return D3D11_COMPARISON_ALWAYS;
            case CompareOperator::None:
            default: DL_ASSERT(false); return D3D11_COMPARISON_NEVER;
            }
        }
    }
}