#include "precomp.h"

#include "game/specialLights.h"
#include "materials/materialManager.h"
#include "lights/lightManager.h"
#include "primitives/bvh.h"
#include "ui/uiManager.h"

Cube::Cube( const float3 pos, const float3 size )
{
	// set cube bounds
	b[0] = pos;
	b[1] = pos + size;
}

float Cube::Intersect( const Ray& ray ) const
{
	const int signX = ray.GetDirection().x < 0;
	const int signY = ray.GetDirection().y < 0;
	const int signZ = ray.GetDirection().z < 0;

	const float3 origin = ray.GetOrigin();
	const float3 reciprocal = ray.GetReciprocalDirection();
	
	float tMin = (b[signX].x - origin.x) * reciprocal.x;
	float tMax = (b[1 - signX].x - origin.x) * reciprocal.x;
	const float tyMin = (b[signY].y - origin.y) * reciprocal.y;
	const float tyMax = (b[1 - signY].y - origin.y) * reciprocal.y;
	
	if (tMin > tyMax || tyMin > tMax) return 1e34f;
	
	tMin = max( tMin, tyMin );
	tMax = min( tMax, tyMax );
	const float tzMin = (b[signZ].z - origin.z) * reciprocal.z;
	const float tzMax = (b[1 - signZ].z - origin.z) * reciprocal.z;
	
	if (tMin > tzMax || tzMin > tMax) return 1e34f;
	
	if ((tMin = max( tMin, tzMin )) > 0) return tMin;

	return 1e34f;
}

bool Cube::Contains( const float3& pos ) const
{
	// test if pos is inside the cube
	return pos.x >= b[0].x && pos.y >= b[0].y && pos.z >= b[0].z &&
		pos.x <= b[1].x && pos.y <= b[1].y && pos.z <= b[1].z;
}

Scene::Scene()
{
	lightManager = new LightManager();
	lightManager->scene = this;
	lightManager->ambientLight = { {1.f}, 0.0f };
	
	lightManager->pointLights.push_back({{-0.3f,0.7f,0.2f},{1.f},1.f});

	lightManager->spotLights.push_back({{-0.6f,0.2f,0.5f}, {-1.f,0.f,0.f},{1.f, 1.f,1.f}, 1.f, 30.f});

	lightManager->areaLights.push_back({{1.5,1.f,0.5f},{1.f},{1.f},1.f,{1.f}});

	Material material;
	material.type = Material::Type::Dielectric;
	material.dielectric.refractiveIndex = 1.5f;
	spheres.push_back(new Sphere{ {0.2f, 0.5f, 1.5f}, 0.2f, {material}, 0xffffff });
	material.type = Material::Type::Glossy;
	material.glossy.fuzz = 0.1f;
	spheres.push_back(new Sphere{ {0.2f, 0.5f, -0.5f}, 0.2f, {material}, 0xffffff });

	bvhSpheres->BuildBVH(spheres);
	
	uiManager = new UIManager();
	uiManager->scene = this;
	materialManager = new MaterialManager();
	// the voxel world sits in a 1x1x1 cube
	cube = Cube( float3( 0, 0, 0 ), float3( 1, 1, 1 ) );
	// initialize the scene using Perlin noise, parallel over z
	grid = static_cast<VoxelData*>(MALLOC64(GRIDSIZE3 * sizeof( VoxelData )));
	memset( grid, 0, GRIDSIZE3 * sizeof( VoxelData ) );

	specialVoxels.resize(6);
	int currentSpecial = 0;
	
	for (int z = 0; z < VOXELAMOUNT; z++)
	{
		for (int y = 0; y < VOXELAMOUNT; y++)
		{
			for (int x = 0; x < VOXELAMOUNT; x++)
			{
				if (x == 0)
				{
					VoxelData data;
					Material material;
					material.type = Material::Type::Mirror;
					data.material = material;
					data.color = 0xffffff;
					data.special = false;
					Set(x,y,z,data);
					continue;
				}
				
				if (y == 0)
				{
					VoxelData data;
					Material material;
					material.type = Material::Type::Diffuse;
					data.material = material;
					data.color = 0x87ceeb;
					data.special = false;
					Set(x,y,z,data);
					continue;
				}

				if (y == 1 && (x == 11 && z == 4 ||
				               x == 11 && z == 8 ||
				               x == 11 && z == 12 ||
				               x == 6 && z == 4 ||
				               x == 6 && z == 8 ||
				               x == 6 && z == 12))
				{
					VoxelData data;
					Material material;
					material.type = Material::Type::Diffuse;
					data.material = material;
					data.color = 0xffffff;
					data.special = true;
					specialVoxels[currentSpecial++] = x + y * GRIDSIZE + z * GRIDSIZE2;
					Set(x,y,z,data);
				}
			}
		}
	}
	specialLights = new SpecialLights(this);
}

int Scene::GetHitVoxelIndex(Ray& ray) const
{
	HitInfo info;
	return FindNearest(ray, info, 0);
}

void Scene::Set(const uint x, const uint y, const uint z, const VoxelData& data) const
{
	grid[x + y * GRIDSIZE + z * GRIDSIZE2] = data;
}

bool Scene::Setup3DDDA( const Ray& ray, DDAState& state ) const
{
	// if ray is not inside the world: advance until it is
	state.t = 0;
	const auto origin = ray.GetOrigin();
	if (!cube.Contains(origin))
	{
		state.t = cube.Intersect( ray );
		if (state.t > 1e33f) return false; // ray misses voxel data entirely
	}
	const auto direction = ray.GetDirection();
	const auto reciprocal = ray.GetReciprocalDirection();
	
	// setup amanatides & woo - assume world is 1x1x1, from (0,0,0) to (1,1,1)
	static constexpr float cellSize = 1.0f / GRIDSIZE;
	state.step = make_int3( 1 - ray.dSign * 2 );
	const float3 posInGrid = GRIDSIZE * (origin + (state.t + 0.00005f) * direction);
	const float3 gridPlanes = (ceilf( posInGrid ) - ray.dSign) * cellSize;
	const int3 P = clamp( make_int3( posInGrid ), 0, GRIDSIZE - 1 );
	state.x = P.x, state.y = P.y, state.z = P.z;
	state.tDelta = cellSize * float3( state.step ) * reciprocal;
	state.tMax = (gridPlanes - origin) * reciprocal;
	// proceed with traversal
	return true;
}

int Scene::FindNearest(Ray& ray, HitInfo& info, int depth) const
{
	int index = -1;
	// setup Amanatides & Woo grid traversal
	DDAState s;
	info.direction = ray.GetDirection();
	info.normal = ray.GetNormal();
	
	if (!Setup3DDDA( ray, s )) return index;
	// start stepping
	while (true)
	{
		index = s.x + s.y * GRIDSIZE + s.z * GRIDSIZE2;
		const auto& cell = grid[index];
		
		if (Math::ValidColor(cell.color))
		{
			ray.length = s.t;
			info.point = ray.GetIntersection();
			info.normal = ray.GetNormal();
			info.color = cell.color;
			info.material = cell.material;
			info.special = cell.special;
			info.specialColor = cell.specialColor;
			return index;
		}
		
		if (s.tMax.x < s.tMax.y)
		{
			if (s.tMax.x < s.tMax.z)
			{
				s.t = s.tMax.x;
				s.x += s.step.x;
				if (s.x >= GRIDSIZE) break;
				s.tMax.x += s.tDelta.x;
			}
			else
			{
				s.t = s.tMax.z;
				s.z += s.step.z;
				if (s.z >= GRIDSIZE) break;
				s.tMax.z += s.tDelta.z;
			}
		}
		else
		{
			if (s.tMax.y < s.tMax.z)
			{
				s.t = s.tMax.y;
				s.y += s.step.y;
				if (s.y >= GRIDSIZE) break;
				s.tMax.y += s.tDelta.y;
			}
			else
			{
				s.t = s.tMax.z;
				s.z += s.step.z;
				if (s.z >= GRIDSIZE) break;
				s.tMax.z += s.tDelta.z;
			}
		}
	}
	
	return index;
	// TODO:
	// - A nested grid will let rays skip empty space much faster.
	// - Coherent rays can traverse the grid faster together.
	// - Perhaps s.X / s.Y / s.Z (the integer grid coordinates) can be stored in a single uint?
	// - Loop-unrolling may speed up the while loop.
}

bool Scene::IsOccluded( const Ray& ray ) const
{
	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA( ray, s )) return false;
	
	// start stepping
	while (s.t < ray.length)
	{
		// if we hit a non-empty cell, the ray is occluded
		const auto color = grid[s.x + s.y * GRIDSIZE + s.z * GRIDSIZE2].color;
		if (Math::ValidColor(color))  return s.t < ray.length;
		
		if (s.tMax.x < s.tMax.y)
		{
			if (s.tMax.x < s.tMax.z)
			{
				if ((s.x += s.step.x) >= GRIDSIZE) return false;
				s.t = s.tMax.x;
				s.tMax.x += s.tDelta.x;
			}
			else
			{
				if ((s.z += s.step.z) >= GRIDSIZE) return false;
				s.t = s.tMax.z;
				s.tMax.z += s.tDelta.z;
			}
		}
		else
		{
			if (s.tMax.y < s.tMax.z)
			{
				if ((s.y += s.step.y) >= GRIDSIZE) return false;
				s.t = s.tMax.y;
				s.tMax.y += s.tDelta.y;
			}
			else
			{
				if ((s.z += s.step.z) >= GRIDSIZE) return false;
				s.t = s.tMax.z;
				s.tMax.z += s.tDelta.z;
			}
		}
	}
	return false;
}





