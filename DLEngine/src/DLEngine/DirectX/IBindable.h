#pragma once
#include "DLEngine/Core/DLWin.h"
#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class IBindable
    {
    public:
        virtual ~IBindable() = default;

        virtual void Bind() = 0;
    };
}
