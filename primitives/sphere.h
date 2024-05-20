#pragma once

class Sphere
{
public:
    Sphere() = default;
    Sphere(const float3& center, float radius, const Material& material, uint color);
    [[nodiscard]] bool HitSphere(Ray& ray, HitInfo& hitInfo, float rayMax = 1e34f) const;
    
    float3 center;
    float radius;
    Material material;
    uint color = 0xffffff;
};
