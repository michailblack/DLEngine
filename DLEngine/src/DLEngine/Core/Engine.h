#pragma once

namespace DLEngine
{
    class Engine
    {
    public:
        // @brief Init core vital systems (e.g. logging, DirectX, etc.)
        static void PreInit();
        
        // @brief Init Engine systems (e.g. Renderer, MeshSystem, etc.)
        static void Init();
        static void Deinit();
    };
}
