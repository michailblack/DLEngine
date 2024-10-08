# DLEngine

This is a basic D3D11 renderer developed as a part of Dragons Lake C++ Render Internship in 2024.

## Main Features

- Physically Based Rendering
- Image Based Lighting
- Shadow mapping for primitive light casters (with PCF for directional lights)
- Instance rendering of opaque geometry
- Basic deferred shading pipeline
- Decal rendering
- Smoke rendering using translucency particles with 6-way lightmaps and motion vectors on CPU-side
- Screen-space collisions using compute shader
- Mesh incineration particles rendering on GPU-side
- HDR + ACES tone mapping
- FXAA (Nvidia's implementation)
- Shader reflection

## Demo

Below are some visuals showcasing key features of DLEngine.

### UI Screenshot
![UI Screenshot](resources/ui.png)

### Smoke Rendering
[![Smoke Rendering](resources/smoke_rendering_thumbnail.png)](resources/smoke_rendering.mkv)

### Decal Rendering
[![Decal Rendering](resources/decal_rendering_thumbnail.jpg)](resources/decal_rendering.mkv)

### Incineration Effect
[![Incineration Effect](resources/incineration_effect_thumbnail.jpg)](resources/incineration_effect.mkv)

## Known Issues

- Ray-mesh intersection takes too long when there are many meshes on the scene