#pragma once

struct Diffuse
{

};

struct Lambert
{

};

struct Mirror
{

};

struct Glossy
{
    float fuzz;
};

struct Dielectric
{
    float refractiveIndex;
};

struct Material
{
    enum class Type
    {
        Diffuse,
        Mirror,
        Glossy,
        Dielectric,
        Lambert
    } type;
    
    union
    {
        Diffuse diffuse;
        Mirror mirror;
        Glossy glossy;
        Dielectric dielectric;
        Lambert lambert;
    };

    uint* pixels = nullptr;
};
