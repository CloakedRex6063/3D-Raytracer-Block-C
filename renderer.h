#pragma once

class Character;

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 HandleVoxelTrace(const HitInfo& hitInfo, const int depth);
	float3 HandleSphereTrace(Ray& ray, HitInfo info, int depth);
	float3 Trace(Ray& ray, int depth);
	void Accumulation(const int& frameIndex, int x, int y, float4 pixel) const;
	void Tick( float deltaTime ) override;
	void UI(float deltaTime) override;
	void Shutdown();
	// input handling
	void MouseUp( int button ) override;
	void MouseDown( int button ) override;
	void MouseMove( int x, int y ) override;
	void MouseWheel( float y ) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp( int key ) { /* implement if you want to handle keys */ }
	void KeyDown( int key ) override;
	// data members
	int2 mousePos;
	int2 prevMousePos;
	float4* accumulator;
	Scene scene;
	Camera* camera;
	Character* character;
};

} // namespace Tmpl8