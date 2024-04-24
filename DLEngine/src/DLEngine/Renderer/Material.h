#pragma once
#include "DLEngine/Core/Application.h"

#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec3.h"

#include "DLEngine/Renderer/Light.h"

class Material
{
public:
    Material(const Math::Vec3& albedo = Math::Vec3 { 1.0f }, float glossiness = 1.0f) noexcept
        : m_Albedo(albedo)
        , m_Glossiness(glossiness)
    {}

    void SetAlbedo(const Math::Vec3& albedo) noexcept { m_Albedo = albedo; }
    void SetSpecularPower(float specularPower) noexcept { m_Glossiness = specularPower; }

    COLORREF CalculateLight(const Math::Ray& ray, const Math::IntersectInfo& intersectionInfo, const Environment& environment) const;

    static COLORREF HDRToCOLORREF(const Math::Vec3& hdrColor, float exposure) noexcept;

private:
    Math::Vec3 m_Albedo;
    float m_Glossiness;
};
