#pragma once

struct HitInfo
{
    float3 point;
    float3 normal;
    float3 direction;
    uint color = 0;
    uint specialColor = 0;
    bool frontFace = false;
    bool special = false;
    
    Material material;
    
    void SetFaceNormal(const float3& outwardNormal)
    {
        frontFace = dot(direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Ray
{
public:
    Ray() = default;
    Ray(const float3& origin, const float3& direction, const float length = 1e34f);

    [[nodiscard]] float3 GetNormal() const;
    [[nodiscard]] float3 GetIntersection() const { return origin + length * direction; }
    [[nodiscard]] float3 GetOrigin() const { return origin; }
    [[nodiscard]] float3 GetDirection() const { return direction; }
    [[nodiscard]] float3 GetReciprocalDirection() const {return 1.f / direction;}

    float length = 1e34f;
    float3 dSign;
    float3 origin;
    float3 direction = float3(0);

    // min3 is used in normal reconstruction.
    __inline static float3 min3(const float3& a, const float3& b)
    {
        return {min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)};
    }
};

