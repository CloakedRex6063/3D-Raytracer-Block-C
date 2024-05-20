#pragma once

struct PointLightData
{
    float3 position;
    float3 color;
    float intensity;
};

struct DirectionalLightData
{
    float3 direction;
    float3 color;
    float intensity;
};

struct SpotLightData
{
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
    float fov;
};

struct AreaLightData
{
    float3 position;
    float3 color;
    float3 direction;
    float intensity;
    float2 size;
};

struct AmbientLightData
{
    float3 color;
    float intensity;
};


struct LightData
{
    enum class Type
    {
        Point,
        Directional,
        Spot,
        Area,
        Ambient
    };

    Type type;
    union
    {
        PointLightData point;
        DirectionalLightData directional;
        SpotLightData spot;
        AreaLightData area;
        AmbientLightData ambient;
    };
};
