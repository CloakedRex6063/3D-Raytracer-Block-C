#include "precomp.h"

#include "game/specialLights.h"
#include "lights/lightManager.h"
#include "materials/materialManager.h"
#include "primitives/bvh.h"
#include "primitives/sphere.h"
#include "ui/uiManager.h"

// YOU GET:
// 1. A fast voxel renderer in plain C/C++
// 2. Normals and voxel colors
// FROM HERE, TASKS COULD BE:							FOR SUFFICIENT
// * Materials:
//   - Reflections and diffuse reflections				<===
//   - Transmission with Snell, Fresnel					<===
//   - Textures, Minecraft-style						<===
//   - Beer's Law
//   - Normal maps
//   - Emissive materials with postproc bloom
//   - Glossy reflections (BASIC)
//   - Glossy reflections (microfacet)
// * Light transport:
//   - Point lights										<===
//   - Spot lights										<===
//   - Area lights										<===
//	 - Sampling multiple lights with 1 ray
//   - Importance-sampling
//   - Image based lighting: sky
// * Camera:
//   - Depth of field									<===
//   - Anti-aliasing									<===
//   - Panini, fish-eye etc.
//   - Post-processing: now also chromatic				<===
//   - Spline cam, follow cam, fixed look-at cam
//   - Low-res cam with CRT shader
// * Scene:
//   - HDR skydome										<===
//   - Spheres											<===
//   - Smoke & trilinear interpolation
//   - Signed Distance Fields
//   - Voxel instances with transform
//   - Triangle meshes (with a BVH)
//   - High-res: nested grid
//   - Procedural art: shapes & colors
//   - Multi-threaded Perlin / Voronoi
// * Various:
//   - Object picking
//   - Ray-traced physics
//   - Profiling & optimization
// * GPU:
//   - GPU-side Perlin / Voronoi
//   - GPU rendering *not* allowed!
// * Advanced:
//   - Ambient occlusion
//   - Denoising for soft shadows
//   - Reprojection for AO / soft shadows
//   - Line lights, tube lights, ...
//   - Bilinear interpolation and MIP-mapping
// * Simple game:										
//   - 3D Arkanoid										<===
//   - 3D Snake?
//   - 3D Tank Wars for two players
//   - Chess
// REFERENCE IMAGES:
// https://www.rockpapershotgun.com/minecraft-ray-tracing
// https://assetsio.reedpopcdn.com/javaw_2019_04_20_23_52_16_879.png
// https://www.pcworld.com/wp-content/uploads/2023/04/618525e8fa47b149230.56951356-imagination-island-1-on-100838323-orig.jpg

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	camera = new Camera();

	// create fp32 rgb pixel buffer to render to
	accumulator = (float4*)MALLOC64( SCRWIDTH * SCRHEIGHT * 16 * camera->numFramesToAccumulate );
	memset( accumulator, 0, SCRWIDTH * SCRHEIGHT * 16 );
	
	/*// try to load a camera
	FILE* f = fopen( "camera.bin", "rb" );
	if (f)
	{
		fread( camera, 1, sizeof( Camera ), f );
		fclose( f );
	}*/
	
	camera->SetCamera();
	prevMousePos = mousePos;
	
	scene.bvhSpheres = new BVHSphere(scene.spheres);
	scene.bvhSpheres->BuildBVH(scene.spheres);
}

int maxDepth = 10;

float3 Renderer::HandleVoxelTrace(const HitInfo& hitInfo, const int depth) 
{
	// Extract information from hitInfo
	const auto intersection = hitInfo.point;
	const auto normal = hitInfo.normal;
	const auto color = Math::GetColorNormalised(hitInfo.color);
	
	// Calculate direct lighting contribution
	auto directLighting = scene.lightManager->CalculateTotalContribution(intersection, normal);

	// Handle scattering
	Ray scattered;
	if (scene.materialManager->Scatter(hitInfo, scattered))
	{
		if (depth + 1 > maxDepth) return scene.skydome.Render(hitInfo.direction);
		directLighting *= color * Trace(scattered, depth + 1);
	}
	else
	{
		if (hitInfo.special && depth == 0)
		{
			directLighting = Math::GetColorNormalised(hitInfo.specialColor);
		}
		else
		{
			directLighting *= color;
		}
	}

	return directLighting;
}

float3 Renderer::HandleSphereTrace(Ray& ray, HitInfo info, const int depth)
{
	auto sphereTrace = float3(0);
	if (scene.bvhSpheres->BeginTraversal(ray, info))
	{
		Ray scattered;
		const auto color = Math::GetColorNormalised(info.color);
		const auto intersection = info.point;
		const auto normal = info.normal;

		const float3 directLighting = scene.lightManager->CalculateTotalContribution(intersection, normal);
			
		if (scene.materialManager->ScatterSphere(info, scattered))
		{
			if (depth + 1 > maxDepth) return scene.skydome.Render(ray.GetDirection());
			sphereTrace = directLighting * color * Trace(scattered, depth + 1);
		}
		else
		{
			sphereTrace = directLighting * color;
		}
	}
	return sphereTrace;
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, const int depth) 
{
	HitInfo info;
	scene.FindNearest(ray, info, depth);
	const auto voxelDistance = ray.length;

	const auto sphereTrace = HandleSphereTrace(ray, info, depth);
	const auto sphereDistance = ray.length;

	// If no intersection occurs, show skydome
	if (sphereDistance == 1e34f && voxelDistance == 1e34f)
	{
		return scene.skydome.Render(ray.GetDirection());
	}

	const auto voxelTrace = HandleVoxelTrace(info, depth);
	if (sphereDistance < voxelDistance)
	{
		return sphereTrace;
	}
	return voxelTrace;
}

void Renderer::Accumulation(const int& frameIndex, int x, int y, const float4 pixel) const
{
	const int numFramesToAccumulate = camera->numFramesToAccumulate;
	// Blend current pixel with accumulated pixel from previous frames
	auto blendedPixel = pixel;
	for (int i = 0; i < numFramesToAccumulate; ++i)
	{
		const int prevFrameIndex = (frameIndex - i + numFramesToAccumulate) % numFramesToAccumulate;
		blendedPixel += accumulator[x + y * SCRWIDTH + prevFrameIndex * SCRWIDTH * SCRHEIGHT];
	}
	blendedPixel *= 1.0f / (numFramesToAccumulate + 1); // Normalize by number of accumulated frames

	// Update accumulated pixel buffer
	accumulator[x + y * SCRWIDTH + frameIndex * SCRWIDTH * SCRHEIGHT] = blendedPixel;

	// Convert accumulated pixel to RGB8 and store it in the screen buffer
	screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&blendedPixel);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime)
{
	static int frameIndex = 0;
	const bool bAccumulate = camera->bAccumulate;

	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			const auto sample = Math::SampleSquare();
			auto ray = camera->GetPrimaryRay(static_cast<float>(x) + sample.x, static_cast<float>(y) + sample.y);
			const auto pixel = float4(Trace(ray, 0), 0);

#ifdef _DEBUG
			// Convert pixel to RGB8 and store it in the screen buffer
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&pixel);
#else
			if (bAccumulate)
			{
				Accumulation(frameIndex, x, y, pixel);
			}
			else
			{
				// Convert pixel to RGB8 and store it in the screen buffer
				screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&pixel);
			}
#endif
		}
	}
	
	//camera->HandleCameraInput(deltaTime);
	/*const auto mouseDelta = mousePos - prevMousePos;
	prevMousePos = mousePos;
	if (mouseHidden)
	{
		camera->UpdateCameraOrientation(static_cast<float>(mouseDelta.x), static_cast<float>(mouseDelta.y));
	}*/
	scene.specialLights->Tick(deltaTime);
	
	
	// Increment frame index
	frameIndex = (frameIndex + 1) % camera->numFramesToAccumulate;
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI(float deltaTime)
{
	ImGui::Begin("Debug Information");
	scene.uiManager->HandleAllUI(deltaTime,*camera);
	ImGui::End();
	
	ImGui::Begin("Score", nullptr,
	             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
	             ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
	             ImGuiWindowFlags_NoFocusOnAppearing);
	ImGui::SetWindowPos({900,0});
	scene.uiManager->HandleScoreUI();
	ImGui::End();

	if (scene.specialLights->bWon)
	{
		ImGui::Begin("Window", nullptr,
		             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		             /*ImGuiWindowFlags_NoBackground |*/ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
		             ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowFontScale(5);
		ImGui::SetWindowPos({(SCRWIDTH / 2) - 100, (SCRHEIGHT / 2) - 50});
		ImGui::Text("Score +1");
		ImGui::End();
	}
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Renderer::Shutdown()
{
	/*// save current camera
	FILE* f = fopen( "camera.bin", "wb" );
	fwrite( camera, 1, sizeof( Camera ), f );
	fclose( f );*/
}

void Renderer::MouseUp(int button)
{
}

void Renderer::MouseDown(int button)
{
	if (button == 0)
	{
		Ray r = camera->GetPrimaryRay(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
		const int hitVoxelIndex = scene.GetHitVoxelIndex(r);
		if (hitVoxelIndex != -1 )
		{
			if (scene.grid[hitVoxelIndex].special && !scene.specialLights->bWon)
			{
				scene.specialLights->CheckSpecialLight(hitVoxelIndex);
			}
		}
	}
}

void Renderer::MouseMove(int x, int y)
{
	static bool first = true;
	if (first)
	{
		prevMousePos.x = x;
		prevMousePos.y = y;
		first = false;
	}
	mousePos.x = x, mousePos.y = y;
}

void Renderer::KeyDown(int key)
{
	static bool photoMode = false;
	if (key == GLFW_KEY_P)
	{
		photoMode = !photoMode;
		camera->TogglePhotoMode(photoMode);
	}
}
