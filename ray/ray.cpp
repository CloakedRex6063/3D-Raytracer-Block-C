#include "precomp.h"
#include "ray.h"
#include "scene.h"

Ray::Ray(const float3& origin, const float3& direction, const float length): length(length), origin(origin), direction(direction)
{
    const uint xSign = *(uint*)&direction.x >> 31;
    const uint ySign = *(uint*)&direction.y >> 31;
    const uint zSign = *(uint*)&direction.z >> 31;
    dSign = (float3(static_cast<float>(xSign) * 2 - 1, static_cast<float>(ySign) * 2 - 1,
                    static_cast<float>(zSign) * 2 - 1) + 1) * 0.5f;
}

float3 Ray::GetNormal() const
{
    // return the voxel normal at the nearest intersection
    const float3 I1 = GetIntersection() * VOXELAMOUNT; // our scene size is (1,1,1), so this scales each voxel to (1,1,1)
    const float3 fG = fracf( I1 );
    const float3 d = min3( fG, 1.0f - fG );
    const float mind = min( min( d.x, d.y ), d.z );
    const float3 sign = dSign * 2 - 1;
    return {mind == d.x ? sign.x : 0, mind == d.y ? sign.y : 0, mind == d.z ? sign.z : 0};
    // TODO:
    // - *only* in case the profiler flags this as a bottleneck:
    // - This function might benefit from SIMD.
}
