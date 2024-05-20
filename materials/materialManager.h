#pragma once
#include "materialData.h"

class Ray;
struct HitInfo;

struct TextureData
{
    int width, height;
    float* pixels;
};

class MaterialManager
{
public:
    MaterialManager();

    bool Scatter(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterDiffuse(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterMirror(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterGlossy(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterDielectric(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterLambert(const HitInfo& hitInfo, Ray& scattered) const;

    bool ScatterSphere(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterDiffuseSphere(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterMirrorSphere(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterGlossySphere(const HitInfo& hitInfo, Ray& scattered) const;
    bool ScatterDielectricSphere(const HitInfo& hitInfo, Ray& scattered) const;
};
