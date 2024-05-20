#include "precomp.h"
#include "materialManager.h"

MaterialManager::MaterialManager()
{
}

bool MaterialManager::Scatter(const HitInfo& hitInfo, Ray& scattered) const
{
    switch (hitInfo.material.type)
    {
    case Material::Type::Diffuse:
        return ScatterDiffuse(hitInfo,scattered);
    case Material::Type::Mirror:
        return ScatterMirror(hitInfo, scattered);
    case Material::Type::Glossy:
        return ScatterGlossy(hitInfo, scattered);
    case Material::Type::Dielectric:
        return ScatterDielectric(hitInfo, scattered);
    case Material::Type::Lambert:
        return ScatterLambert(hitInfo, scattered);
    }
    return false;
}

bool MaterialManager::ScatterDiffuse(const HitInfo& hitInfo, Ray& scattered) const
{
    return false;
}

bool MaterialManager::ScatterMirror(const HitInfo& hitInfo, Ray& scattered) const
{
    auto normal = hitInfo.normal;
    const auto incidentDirection = hitInfo.direction;
    if (dot(incidentDirection, normal) > 0)
    {
        // If the incident ray and the normal are facing the same direction,
        // flip the normal.
        normal = -normal;
    }
    const auto reflected = Math::Reflect(incidentDirection, normal);
    scattered = Ray(hitInfo.point + EPSILON * reflected, reflected);
    return true;
}

bool MaterialManager::ScatterGlossy(const HitInfo& hitInfo, Ray& scattered) const
{
    auto normal = hitInfo.normal;
    const float3 incidentDirection = hitInfo.direction;
    if (dot(incidentDirection, normal) > 0)
    {
        // If the incident ray and the normal are facing the same direction,
        // flip the normal.
        normal = -normal;
    }
    
    const auto reflected = Math::Reflect(incidentDirection, normal);

    // Perturb the reflected direction using importance sampling (cosine-weighted)
    const float3 perturbedReflected = reflected + hitInfo.material.glossy.fuzz * Math::CosineWeightedSample(normal);

    scattered = Ray(hitInfo.point + EPSILON * perturbedReflected, perturbedReflected);
    return true;
}

bool MaterialManager::ScatterDielectric(const HitInfo& hitInfo, Ray& scattered) const
{
    const auto normal = hitInfo.normal;
    const auto incidentDirection = hitInfo.direction;
    const auto refractiveIndex = hitInfo.material.dielectric.refractiveIndex;
    const float refractionRatio = hitInfo.frontFace ? (1.0f / refractiveIndex) : refractiveIndex;

    const float3 unitIncidentDirection = normalize(incidentDirection);
    const float cosTheta = fminf(dot(-unitIncidentDirection, normal), 1.f);
    const float sinTheta = sqrtf(1.f - cosTheta * cosTheta);

    float3 direction;

    if (refractionRatio * sinTheta > 1.0 || Math::Reflectance(cosTheta, refractionRatio) > RandomFloat())
    {
        direction = Math::Reflect(unitIncidentDirection, normal);
    } else
    {
        direction = Math::Refract(unitIncidentDirection, normal, refractionRatio);
    }

    scattered = Ray(hitInfo.point + EPSILON * direction, direction);
    return true;
}

bool MaterialManager::ScatterLambert(const HitInfo& hitInfo, Ray& scattered) const
{
    auto normal = hitInfo.normal;
    const auto incidentDirection = hitInfo.direction;
    const auto intersection = hitInfo.point;
    if (dot(incidentDirection, normal) > 0)
    {
        // If the incident ray and the normal are facing the same direction,
        // flip the normal.
        normal = -normal;
    }
    const auto target = intersection + normal + Math::RandomUnitVector();
    scattered = Ray(intersection + EPSILON * target, target - intersection);
    return true;
}

bool MaterialManager::ScatterSphere(const HitInfo& hitInfo, Ray& scattered) const
{
    switch (hitInfo.material.type)
    {
        case Material::Type::Diffuse:
            return ScatterDiffuseSphere(hitInfo, scattered);
        case Material::Type::Mirror:
            return ScatterMirrorSphere(hitInfo, scattered);
        case Material::Type::Glossy:
            return ScatterGlossySphere(hitInfo, scattered);
        case Material::Type::Dielectric:
            return ScatterDielectricSphere(hitInfo, scattered);
        case Material::Type::Lambert:
            return ScatterDiffuseSphere(hitInfo,scattered);
    }
    return false;
}


bool MaterialManager::ScatterDiffuseSphere(const HitInfo& hitInfo, Ray& scattered) const
{
    //const auto normal = hitInfo.normal;
    //const auto scatterDirection = normal + Math::CosineWeightedSample(normal);
    //scattered = RayN(hitInfo.point, scatterDirection);
    return false;
}

bool MaterialManager::ScatterMirrorSphere(const HitInfo& hitInfo, Ray& scattered) const
{
    auto normal = hitInfo.normal;
    const auto incidentDirection = hitInfo.direction;
    if (dot(incidentDirection, normal) > 0)
    {
        // If the incident ray and the normal are facing the same direction,
        // flip the normal.
        normal = -normal;
    }
    const auto reflected = Math::Reflect(incidentDirection, normal);
    scattered = Ray(hitInfo.point, reflected);
    return true;
}

bool MaterialManager::ScatterGlossySphere(const HitInfo& hitInfo, Ray& scattered) const
{
    const float3 reflected = Math::Reflect(hitInfo.direction, hitInfo.normal);
    const float3 perturbedReflected = reflected + hitInfo.material.glossy.fuzz * Math::RandomUnitVector();
    scattered = Ray(hitInfo.point, perturbedReflected);
    
    return true;
}

bool MaterialManager::ScatterDielectricSphere(const HitInfo& hitInfo, Ray& scattered) const
{
    const float ir = hitInfo.material.dielectric.refractiveIndex;
    const auto refractionRatio = hitInfo.frontFace ? (1.f/ir) : ir;
    const auto normal = hitInfo.normal;
    const float3 unitDirection = normalize(hitInfo.direction);
    
    const auto cosTheta = fmin(dot(-unitDirection, normal), 1.f);
    const auto sinTheta = sqrt(1.f - cosTheta*cosTheta);

    // Total internal reflection
    const bool cannotRefract = refractionRatio * sinTheta > 1.0;
    float3 direction;

    // Schlick's approximation
    if (cannotRefract || Math::Reflectance(cosTheta, refractionRatio) > RandomFloat())
    {
        direction = Math::Reflect(unitDirection, normal);
    }
    else
    {
        direction = Math::Refract(unitDirection, normal, refractionRatio);
    }
    
    scattered = Ray(hitInfo.point, direction);
    return true;
}

