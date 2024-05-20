#include "precomp.h"
#include "sphere.h"

Sphere::Sphere(const float3& center, const float radius, const Material& material, const uint color): center(center), radius(radius), material(material), color(color)
{

}

bool Sphere::HitSphere(Ray& ray, HitInfo& hitInfo, float rayMax) const
{
    const auto originToCenter = ray.GetOrigin() - center;
    const auto a = sqrLength(ray.GetDirection());
    const auto halfB = dot(originToCenter, ray.GetDirection());
    const auto c = sqrLength(originToCenter) - radius * radius;
    const auto discriminant = halfB * halfB - a * c;

    hitInfo.direction = ray.GetDirection();
    
    // If the discriminant is negative, there is no intersection
    if (discriminant < 0) return false;

    const auto discriminantSqrt = sqrt(discriminant);

    auto root = (-halfB - discriminantSqrt) / a;
    if (!Math::InRange(EPSILON, rayMax, root))
    {
        root = (-halfB + discriminantSqrt) / a;
        if (!Math::InRange(EPSILON, rayMax, root)) return false;
    }

    ray.length = root;
    hitInfo.point = ray.GetIntersection();
    hitInfo.SetFaceNormal((hitInfo.point - center) / radius);
    hitInfo.material = material;
    hitInfo.color = color;
    return true;
}
