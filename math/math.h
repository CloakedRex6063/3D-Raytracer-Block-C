#pragma once
#include <random>

#include "materials/materialManager.h"
#include "primitives/sphere.h"

inline std::random_device rd;
inline std::mt19937 gen(rd());

class Math
{
public:
    static float Clamp(float a, float min, float max)
    {
        return fmaxf(min, fminf(a, max));
    }
    
    static bool InRange(float min, float max, float value)
    {
        return value >= min && value <= max;
    }

    static float3 RandomPointInDisk()
    {
        while (true)
        {
            auto p = float3(RandomFloatRange(-1,1), RandomFloatRange(-1,1), 0);
            if (sqrLength(p) < 1) return p;
        }
    }

    static __m256 RandomPointInDisk8()
    {
        // Generate random floats for x and y coordinates of 8 points
        const __m256 randomX = _mm256_set_ps(RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1));
        const __m256 randomY = _mm256_set_ps(RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1),
                                             RandomFloatRange(-1, 1), RandomFloatRange(-1, 1));

        // Compute squared lengths of points
        const __m256 sqrLengths = _mm256_add_ps(_mm256_mul_ps(randomX, randomX),
                                                _mm256_mul_ps(randomY, randomY));

        // Mask out points outside the unit disk
        const __m256 mask = _mm256_cmp_ps(sqrLengths, _mm256_set1_ps(1.0f), _CMP_LE_OS);

        // Apply mask to x and y coordinates
        const __m256 maskedX = _mm256_and_ps(mask, randomX);
        const __m256 maskedY = _mm256_and_ps(mask, randomY);

        return _mm256_blendv_ps(maskedX, maskedY, mask);
    }

    
    static float3 Reflect(const float3& incident, const float3& normal)
    {
        return incident - 2 * dot(incident, normal) * normal;
    }
    
    static float3 Refract(const float3& incident, const float3& normal, const float eta)
    {
        const float nDotI = dot(normal, incident);
        const float k = 1.0f - eta * eta * (1.0f - nDotI * nDotI);

        if (k < 0.0f) return {0.f}; // Total internal reflection

        return eta * incident - (eta * nDotI + sqrtf(k)) * normal;
    }

    static float DegreesToRadians(const float degrees)
    {
        return degrees * (PI / 180.f);
    }

    static float3 DegreesToRadians(const float3& degrees)
    {
        return {degrees.x * (PI / 180.f), degrees.y * (PI / 180.f), degrees.z * (PI / 180.f)};
    }

    static float RadiansToDegrees(const float radians)
    {
        return radians * (180.f / PI);
    }

    static float MaxComponent(const float3& v)
    {
        return fmaxf(fmaxf(v.x, v.y), v.z);
    }

    static float MinComponent(const float3& v)
    {
        return fminf(fminf(v.x, v.y), v.z);
    }

    static float3 Min(const float3& a, const float3& b)
    {
        return {fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)};
    }

    static float3 Max(const float3& a, const float3& b)
    {
        return {fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)};
    }

    static float2 SampleSquare()
    {
        return {RandomFloatRange(-0.5, 0.5), RandomFloatRange(-0.5, 0.5)};
    }

    static float3 RandomPointOnSquare(const float3& center, const float2& size)
    {
        const float x = RandomFloatRange(-size.x, size.x);
        const float y = RandomFloatRange(-size.y, size.y);
        return center + float3{x, y, 0};
    }
    
    static float3 RandomPointOnSphere(const float3& center)
    {
        const float theta = RandomFloatRange(0.f, 2.f * PI);
        const float phi = RandomFloatRange(0.f, PI);
        const float x = sinf(phi) * cosf(theta);
        const float y = sinf(phi) * sinf(theta);
        const float z = cosf(phi);
        return center + float3{x, y, z};
    }

    // Function to generate a deterministic direction in a hemisphere, weighted by cosine of the angle
    static float3 CosineWeightedSampleHemisphere(const float3& normal)
    {
        float u1 = 0.5f;
        float u2 = 0.5f;

        // Calculate spherical coordinates
        const float theta = 2.0f * PI * u1;
        const float phi = acos(2.0f * u2 - 1.0f);

        // Convert to Cartesian coordinates
        const float x = cos(theta) * sin(phi);
        const float y = sin(theta) * sin(phi);
        const float z = cos(phi);

        // Transform the sampled direction to the hemisphere around the normal
        float3 tangent = (fabs(normal.x) > 0.1f) ? float3{0, 1, 0} : float3{1, 0, 0};
        float3 bitangent = cross(normal, tangent);
        float3 worldDirection = tangent * x + bitangent * y + normal * z;

        return worldDirection;
    }
    
    static float3 RandomUnitVector()
    {
        const float x = RandomFloatRange(-1.f, 1.f);
        const float y = RandomFloatRange(-1.f, 1.f);
        const float z = RandomFloatRange(-1.f, 1.f);
        return {x,y,z};
    }

    static float3 RandomInUnitSphere()
    {
        while (true)
        {
            const auto value = RandomUnitVector();
            if (sqrLength(value) < 1) return value;
        }
    }
    
    // Function to sample a direction using cosine-weighted distribution
    static float3 CosineWeightedSample(const float3& normal)
    {
        float r1 = RandomFloat(); // Random number between 0 and 1
        float r2 = RandomFloat(); // Random number between 0 and 1

        // Use polar coordinates to create a vector with a cosine-weighted distribution
        float phi = 2 * PI * r1;
        float cosTheta = sqrt(1 - r2); // Cosine of the polar angle

        // Convert polar coordinates to Cartesian coordinates
        float x = cos(phi) * 2 * sqrt(r2 * (1 - r2));
        float y = sin(phi) * 2 * sqrt(r2 * (1 - r2));
        float z = cosTheta;

        // Create a vector in the local coordinate system
        float3 localDirection = make_float3(x, y, z);

        // Transform the local direction to world space
        float3 tangent, bitangent;
        CreateCoordinateSystem(normal, tangent, bitangent);

        // Transform local direction to world space
        return normalize(tangent * localDirection.x + bitangent * localDirection.y + normal * localDirection.z);
    }

    static void CreateCoordinateSystem(const float3& normal, float3& tangent, float3& bitangent)
    {
        // Assuming normal is normalized
        if (fabs(normal.x) > fabs(normal.y))
            tangent = normalize(make_float3(normal.z, 0, -normal.x) / sqrt(normal.x * normal.x + normal.z * normal.z));
        else
            tangent = normalize(make_float3(0, -normal.z, normal.y) / sqrt(normal.y * normal.y + normal.z * normal.z));

        bitangent = cross(normal, tangent);
    }
    
    
    static int RandomIntRange(const int min, const int max)
    {
        std::uniform_int_distribution distribution(min, max);
        return distribution(gen);
    }
    
    static float RandomFloatRange(const float min, const float max)
    {
        return min + RandomFloat() * (max - min);
    }

    static bool NearZero(const float3& v)
    {
        constexpr float s = 1e-8f;
        return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
    }

    static float Reflectance(const float cosine, const float refractionRatio)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refractionRatio) / (1 + refractionRatio);
        r0 *= r0;
        return r0 + (1.f - r0) * pow((1.f - cosine), 5.f);
    }
    
    // ------------------------------
    // Color
    // ------------------------------
    static float3 GetColorNormalised(const uint color)
    {
        const uint r = (color >> 16) & 0xFF; // Extract the lowest 8 bits
        const uint g = (color >> 8) & 0xFF; // Shift right by 8 bits to get the next 8 bits, and mask to get the lowest 8 bits
        const uint b = color & 0xFF; // Shift right by 16 bits to get the next 8 bits, and mask to get the lowest 8 bits
        return {r / 255.f, g / 255.f, b / 255.f};
    }

    static uint GetColor(const float3& color)
    {
        const uint r = static_cast<uint>(255.999f * color.x);
        const uint g = static_cast<uint>(255.999f * color.y);
        const uint b = static_cast<uint>(255.999f * color.z);
        return (r << 16) | (g << 8) | b;
    }

    static bool ValidColor(const uint color)
    {
        return color != 0;
    }

    static uint GetMaterialIndex(const uint voxel)
    {
        return voxel >> 24;
    }
};

