#include "precomp.h"
#include "bvh.h"

BVHSphere::BVHSphere(const vector<Sphere*>& spheres)
{
    root = BuildBVH(spheres);
}

BVHSphereNode* BVHSphere::BuildBVH(const std::vector<Sphere*>& spheres)
{
    if (spheres.empty()) return nullptr;

    // Create a new node
    const auto node = new BVHSphereNode();
    // If there is only one sphere, assign it to the node
    if (spheres.size() == 1) node->sphere = spheres[0];

    // Otherwise, split the spheres and build the BVH
    else
    {
        vector<Sphere*> left, right;
        SplitSpheres(spheres, left, right);
        node->left = BuildBVH(left);
        node->right = BuildBVH(right);
    }
    return node;
}

void BVHSphere::SplitSpheres(const vector<Sphere*>& spheres, vector<Sphere*>& left, vector<Sphere*>& right) const
{
    const aabb aabb = GetAABB(spheres);
    const auto size = spheres.size();
    const auto axis = aabb.LongestAxis();

    // Sort the spheres along the longest axis
    vector<Sphere*> sortedSpheres = spheres;
    std::sort(sortedSpheres.begin(), sortedSpheres.end(), [axis](Sphere* a, Sphere* b)
    {
        return a->center[axis] < b->center[axis];
    });

    // Split the spheres in half
    const auto mid = size / 2;
    left = vector(sortedSpheres.begin(), sortedSpheres.begin() + mid);
    right = vector(sortedSpheres.begin() + mid, sortedSpheres.end());
}

aabb BVHSphere::GetAABB(const vector<Sphere*>& spheres) const
{
    float3 min = spheres[0]->center - float3(spheres[0]->radius);
    float3 max = spheres[0]->center + float3(spheres[0]->radius);
    
    for (const auto& sphere : spheres)
    {
        const auto sphereMin = sphere->center - float3(sphere->radius);
        const auto sphereMax = sphere->center + float3(sphere->radius);

        min.x = std::min(min.x, sphereMin.x);
        min.y = std::min(min.y, sphereMin.y);
        min.z = std::min(min.z, sphereMin.z);

        max.x = std::max(max.x, sphereMax.x);
        max.y = std::max(max.y, sphereMax.y);
        max.z = std::max(max.z, sphereMax.z);
    }

    return {min, max};
}

bool BVHSphere::Traverse(BVHSphereNode* node, Ray& ray, HitInfo& hitInfo) const
{
    if (!node) return false;

    // If the node is a leaf, check for intersection
    if (node->sphere)
    {
        return node->sphere->HitSphere(ray, hitInfo);
    }

    // If the node is not a leaf, check both children
    return Traverse(node->left, ray,hitInfo) || Traverse(node->right, ray, hitInfo);
}

bool BVHSphere::BeginTraversal(Ray& ray, HitInfo& hitInfo) const
{
    return Traverse(root, ray, hitInfo);
}