#pragma once

// high level settings
// #define TWOLEVEL
#define VOXELAMOUNT 16 // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!
// #define USE_SIMD
// #define USE_FMA3
// #define SKYDOME
// #define WHITTED
// #define DOF

// low-level / derived
#define VOXELAMOUNT2	(VOXELAMOUNT * VOXELAMOUNT)


#ifdef TWOLEVEL
// feel free to replace with whatever suits your two-level implementation,
// should you chose this challenge.
#define BRICKSIZE	8
#define BRICKSIZE2	(BRICKSIZE*BRICKSIZE)
#define BRICKSIZE3	(BRICKSIZE*BRICKSIZE*BRICKSIZE) 
#define GRIDSIZE	(WORLDSIZE/BRICKSIZE)
#define VOXELSIZE	(1.0f/WORLDSIZE)
#else
#define GRIDSIZE	VOXELAMOUNT
#endif
#define GRIDSIZE2	(GRIDSIZE*GRIDSIZE)
#define GRIDSIZE3	(GRIDSIZE*GRIDSIZE*GRIDSIZE)

#include "lights/skydome.h"
#include "primitives/sphere.h"

class SpecialLights;
class BVHSphere;
class MaterialManager;
class LightManager;
class UIManager;

struct VoxelData
{
    Material material;
    uint color = 0;
    uint specialColor = 0;
    bool special = false;
};

namespace Tmpl8
{
    class Cube
    {
    public:
        Cube() = default;
        Cube(const float3 pos, const float3 size);
        [[nodiscard]] float Intersect(const Ray& ray) const;
        [[nodiscard]] bool Contains(const float3& pos) const;
        float3 b[2];
    };
    
    class Scene
    {
    public:
        struct DDAState
        {
            int3 step; // 16 bytes
            uint x, y, z; // 12 bytes
            float t; // 4 bytes
            float3 tDelta;
            float3 tMax;
        };

        Scene();
        [[nodiscard]] int GetHitVoxelIndex(Ray& ray) const;
        int FindNearest(Ray& ray, HitInfo& info, int depth) const;
        [[nodiscard]] bool IsOccluded(const Ray& ray) const;
        void Set(const uint x, const uint y, const uint z, const VoxelData& data) const;
        VoxelData* grid;
        Cube cube;
        float size;
        LightManager* lightManager;
        UIManager* uiManager;
        MaterialManager* materialManager;
        Skydome skydome;
        vector<Sphere*> spheres;
        BVHSphere* bvhSpheres;
        vector<uint> specialVoxels;
        SpecialLights* specialLights;

    private:
        bool Setup3DDDA(const Ray& ray, DDAState& state) const;
    };
}
