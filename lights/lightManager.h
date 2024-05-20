#pragma once
#include "lightData.h"

namespace Tmpl8
{
    class Scene;
}

class LightManager
{
public:
    [[nodiscard]] float3 CalculateAmbientLight() const;
    [[nodiscard]] float3 CalculatePointLight(const PointLightData& light, const float3& point) const;
    [[nodiscard]]float3 CalculateDirectionalLight(const DirectionalLightData& light) const;
    [[nodiscard]] float3 CalculateSpotLight(const SpotLightData& light, const float3& point) const;
    [[nodiscard]] float3 CalculateAreaLight(const AreaLightData& light, const float3& point) const;

    [[nodiscard]] float3 CalculateTotalContribution(const float3& point, const float3& normal) const;
    [[nodiscard]] float3 CalculateStochasticTotalContribution(const float3& point, const float3& normal) const;
    [[nodiscard]] float CastShadow(const float3& intersection, const float3& normal, const float3& direction, float distance = FLT_MAX) const;

    vector<PointLightData> pointLights;
    vector<SpotLightData> spotLights;
    vector<DirectionalLightData> directionalLights;
    vector<AreaLightData> areaLights;
    AmbientLightData ambientLight;
    Scene* scene;
    bool bStochastic = false;
    float softShadowAmount = 0.7f;
};
