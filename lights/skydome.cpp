#include "precomp.h"
#include "skydome.h"

Skydome::Skydome()
{
    // Load Skydome From File
    pixels = stbi_loadf("assets/stormHdr.hdr", &width, &height, &bpp, 0); // Skydome Source: https://hdri-haven.com/hdri/rock-formations
    for (int i = 0; i < width * height * 3; i++)
        pixels[i] = sqrtf(pixels[i]); // Gamma Adjustment for Reduced HDR Range
}

float3 Skydome::Render(float3 direction) const
{
    const float3 dir = normalize(direction);

    // Sample Sky
    const uint u = static_cast<uint>(static_cast<float>(width) * atan2f(dir.z, dir.x) * INV2PI - 0.5f);
    const uint v = static_cast<uint>(static_cast<float>(height) * acosf(dir.y) * INVPI - 0.5f);
    const uint skyIndex = (u + v * width) % (width * height);
    return 0.65f * float3(pixels[skyIndex * 3], pixels[skyIndex * 3 + 1], pixels[skyIndex * 3 + 2]);
}
