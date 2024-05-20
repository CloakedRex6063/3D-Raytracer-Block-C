#pragma once

struct BVHSphereNode
{
    BVHSphereNode* left = nullptr;
    BVHSphereNode* right = nullptr;
    Sphere* sphere = nullptr;
};

class BVHSphere
{
public:
    BVHSphere(const vector<Sphere*>& spheres);
    BVHSphereNode* BuildBVH(const vector<Sphere*>& spheres);
    void SplitSpheres(const vector<Sphere*>& spheres, vector<Sphere*>& left, vector<Sphere*>& right) const;
    aabb GetAABB(const vector<Sphere*>& spheres) const;
    
    bool Traverse(BVHSphereNode* node, Ray& ray, HitInfo& hitInfo) const;
    bool BeginTraversal(Ray& ray, HitInfo& hitInfo) const;

private:
    BVHSphereNode* root = nullptr;
};

