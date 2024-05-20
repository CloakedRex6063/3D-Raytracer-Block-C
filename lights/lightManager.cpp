#include "precomp.h"
#include "lightManager.h"

float3 LightManager::CalculateAmbientLight() const
{
    return ambientLight.color * ambientLight.intensity;
}

float3 LightManager::CalculatePointLight(const PointLightData& light, const float3& point) const
{
    return light.color * light.intensity / length(light.position - point);
}

float3 LightManager::CalculateDirectionalLight(const DirectionalLightData& light) const
{
    return light.color * light.intensity;
}

float3 LightManager::CalculateSpotLight(const SpotLightData& light, const float3& point) const
{
    const auto direction = normalize(light.position - point);
    const auto angle = dot(direction, light.direction);
    if (angle < cos(Math::DegreesToRadians(light.fov))) return {0};
    return light.color * light.intensity / length(light.position - point);
}

float3 LightManager::CalculateAreaLight(const AreaLightData& light, const float3& point) const
{
    const auto size = light.size;
    const auto position = Math::RandomPointOnSquare(light.position, size);
    const auto direction = normalize(position - point);
    const auto angle = dot(direction, light.direction);
    if (angle < 0) return {0};
    return light.color * light.intensity / length(position - point);
}

float3 LightManager::CalculateTotalContribution(const float3& point, const float3& normal) const
{
    auto totalDiffuse = float3{0};
    totalDiffuse += CalculateAmbientLight();

    const int lightCount = static_cast<int>(pointLights.size() + directionalLights.size() + spotLights.size() +
        areaLights.size());
    if (bStochastic && lightCount > 0)
    {
        totalDiffuse += CalculateStochasticTotalContribution(point, normal);
    }

    else
    {
        auto shadowIntensity = 0.f;
        // Handle lighting for point lights

        for (auto& pointLight : pointLights)
        {
            auto diffuseVal = CalculatePointLight(pointLight, point);
            const auto direction = normalize(pointLight.position - point);
            const auto distance = length(pointLight.position - point);
            shadowIntensity = CastShadow(point, normal, direction, distance);
            const float diffuseIntensity = max(dot(normal, direction), 0.f);
            totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        }

        for (auto& directionalLight : directionalLights)
        {
            const auto diffuseVal = CalculateDirectionalLight(directionalLight);
            const auto direction = normalize(-directionalLight.direction);
            shadowIntensity = CastShadow(point, normal, direction);
            const float diffuseIntensity = max(dot(normal, direction), 0.f);
            totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        }

        for (auto& spotLight : spotLights)
        {
            auto diffuseVal = CalculateSpotLight(spotLight, point);
            const auto direction = normalize(point - spotLight.position);
            const auto distance = length( point - spotLight.position);
            shadowIntensity = CastShadow(point, normal, direction, distance);
            totalDiffuse += diffuseVal * shadowIntensity;
        }

        for (auto& areaLight : areaLights)
        {
            auto diffuseVal = CalculateAreaLight(areaLight, point);
            const auto direction = normalize(areaLight.position - point);
            const auto distance = length(areaLight.position - point);
            shadowIntensity = CastShadow(point, normal, direction, distance);
            const float diffuseIntensity = max(dot(normal, direction), 0.f);
            totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        }
    }
    
    return totalDiffuse;
}

float3 LightManager::CalculateStochasticTotalContribution(const float3& point, const float3& normal) const
{
    const int totalLights = static_cast<int>(pointLights.size() + directionalLights.size() + spotLights.size() +
        areaLights.size());
    int lightSelection = Math::RandomIntRange(0, totalLights);
    int lightIndex;
    auto totalDiffuse = float3{0};
    float shadowIntensity;

    if (lightSelection < static_cast<int>(pointLights.size()))
    {
        lightIndex = lightSelection;
        auto& pointLight = pointLights[lightIndex];
        const auto diffuseVal = CalculatePointLight(pointLight, point);
        const auto direction = normalize(pointLight.position - point);
        const auto distance = length(pointLight.position - point);
        shadowIntensity = CastShadow(point, normal, direction, distance);
        const float diffuseIntensity = max(dot(normal, direction), 0.f);
        totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        return totalDiffuse * static_cast<float>(totalLights);
    }
    
    lightSelection -= static_cast<int>(pointLights.size());
    if (lightSelection < static_cast<int>(directionalLights.size()))
    {
        lightIndex = lightSelection;
        auto& directionalLight = directionalLights[lightIndex];
        const auto diffuseVal = CalculateDirectionalLight(directionalLight);
        const auto direction = normalize(directionalLight.direction);
        shadowIntensity = CastShadow(point, normal, direction);
        const float diffuseIntensity = max(dot(normal, direction), 0.f);
        totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        return totalDiffuse * static_cast<float>(totalLights);
    }
    
    lightSelection -= static_cast<int>(directionalLights.size());
    if (lightSelection < static_cast<int>(spotLights.size()))
    {
        lightIndex = lightSelection;
        auto& spotLight = spotLights[lightIndex];
        const auto diffuseVal = CalculateSpotLight(spotLight, point);
        const auto direction = normalize(point - spotLight.position);
        const auto distance = length( point - spotLight.position);
        shadowIntensity = CastShadow(point, normal, direction, distance);
        totalDiffuse += diffuseVal * shadowIntensity;
        return totalDiffuse * static_cast<float>(totalLights);
    }
    
    lightSelection -= static_cast<int>(spotLights.size());
    if (lightSelection < static_cast<int>(areaLights.size()))
    {
        lightIndex = lightSelection;
        auto& areaLight = areaLights[lightIndex];
        const auto diffuseVal = CalculateAreaLight(areaLight, point);
        const auto direction = normalize(areaLight.position - point);
        const auto distance = length(areaLight.position - point);
        shadowIntensity = CastShadow(point, normal, direction, distance);
        const float diffuseIntensity = max(dot(normal, direction), 0.f);
        totalDiffuse += diffuseVal * diffuseIntensity * shadowIntensity;
        return totalDiffuse * static_cast<float>(totalLights);
    }
    return totalDiffuse;
}

float LightManager::CastShadow(const float3& intersection, const float3& normal, const float3& direction,
                               float distance) const
{
    if (dot(normal,direction) < 0) return 0;
    float shadowIntensity = 0.f;
    const auto shadowRayOrigin = intersection + normal * EPSILON;
    const auto shadowRayDirection = direction + Math::RandomUnitVector() * softShadowAmount;

    const Ray shadowRay{shadowRayOrigin, shadowRayDirection, distance};
    if (!scene->IsOccluded(shadowRay)) shadowIntensity += 1.f;
    return shadowIntensity;
}
