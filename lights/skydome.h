#pragma once

class Skydome
{
public:
    Skydome();
    [[nodiscard]] float3 Render(float3 direction) const;

private:
    int width, height, bpp;
    float* pixels;
};
