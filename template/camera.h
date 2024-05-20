#pragma once

// default screen resolution
#define SCRWIDTH	1280
#define SCRHEIGHT	720
//#define FULLSCREEN
//#define DOUBLESIZE

namespace Tmpl8
{

class Camera
{
public:
	Camera()
	{
		// setup a basic view frustum
		/*camPos = float3( 0, 0, -2 );
		camTarget = float3( 0, 0, -1 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );*/
	}

	void SetCamera()
	{
		auto tmpUp = float3(0, 1, 0);
		auto forward = normalize(camTarget - camPos);
		const float3 right = normalize(cross(tmpUp, forward));
		const float3 up = normalize(cross(forward, right));

		topLeft = camPos + 2 * forward - aspect * right + up;
		topRight = camPos + 2 * forward + aspect * right + up;
		bottomLeft = camPos + 2 * forward - aspect * right - up;
	}

	void TogglePhotoMode(bool enabled)
	{
		if (enabled)
		{
			apertureRadius = 0.06f;
		}
		else
		{
			apertureRadius = 0;
		}
	}

	[[nodiscard]] Ray GetPrimaryRay( const float x, const float y ) const
	{
		const float u = x * (1.0f / SCRWIDTH);
		const float v = y * (1.0f / SCRHEIGHT);
    
		const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);

		// Offset the ray origin within the aperture
		const float3 apertureOffset = Math::RandomPointInDisk() * apertureRadius;

		// Calculate the point on the focal plane
		const float3 focalPoint = camPos + (P - camPos) * (focalLength / length(P - camPos));

		// Ray direction towards the focal point
		float3 direction = normalize(focalPoint - (camPos + apertureOffset));

		// Final ray origin, considering the offset
		float3 rayOrigin = camPos + apertureOffset;

		return {rayOrigin, direction};
	}

	[[nodiscard]] float3 GetForwardVector() const
	{
		return normalize(camTarget - camPos);
	}

	[[nodiscard]] float3 GetRightVector() const
	{
		const float3 tmpUp(0, 1, 0);
		return normalize(cross(tmpUp, normalize(camTarget - camPos)));
	}

	void Move(const float t, const float3 direction)
	{
		// Calculate new forward vector based on yaw and pitch
		float3 forward = normalize(camTarget - camPos);

		// Calculate right and up vectors
		const float3 tmpUp(0, 1, 0);

		const float speed = camSpeed * t;

		camPos += speed * direction;

		camTarget = camPos + forward;

		forward = normalize(camTarget - camPos);
		const float3 right = normalize(cross(tmpUp, forward));
		const float3 up = normalize(cross(forward, right));

		topLeft = camPos + 2 * forward - aspect * right + up;
		topRight = camPos + 2 * forward + aspect * right + up;
		bottomLeft = camPos + 2 * forward - aspect * right - up;
	}

	void Look(float yaw, float pitch)
	{
		yaw = Math::DegreesToRadians(yaw);
		pitch = Math::DegreesToRadians(pitch);
		// Apply yaw and pitch rotations
		float3 newForward;
		newForward.x = cos(yaw) * cos(pitch);
		newForward.y = sin(pitch);
		newForward.z = sin(yaw) * cos(pitch);
		const auto forward = normalize(newForward);

		// Calculate right and up vectors
		const float3 tmpUp(0, 1, 0);
		const float3 right = normalize(cross(tmpUp, forward));
		const float3 up = normalize(cross(forward, right));

		// Update camera position based on the new orientation
		camTarget = camPos + forward;

		// Update corners of the view frustum
		topLeft = camPos + 2 * forward - aspect * right + up;
		topRight = camPos + 2 * forward + aspect * right + up;
		bottomLeft = camPos + 2 * forward - aspect * right - up;
	}

	void HandleCameraInput(const float t)
	{
		if (GetAsyncKeyState('W')) Move(t, GetForwardVector());
		if (IsKeyDown(GLFW_KEY_S)) Move(t, -GetForwardVector());
		if (IsKeyDown(GLFW_KEY_A)) Move(t, -GetRightVector());
		if (IsKeyDown(GLFW_KEY_D)) Move(t, GetRightVector());
		if (IsKeyDown(GLFW_KEY_SPACE)) Move(t, float3(0, 1, 0));
		if (IsKeyDown(GLFW_KEY_LEFT_CONTROL)) Move(t, float3(0, -1, 0));
	}

	void UpdateCameraOrientation(float deltaX, float deltaY)
	{
		// Sensitivity of mouse movement
		constexpr float sensitivity = 0.1f;

		// Update yaw and pitch angles based on mouse movement
		const float yawChange = -deltaX * sensitivity;
		const float pitchChange = -deltaY * sensitivity;

		// Define limits for pitch to avoid flipping the camera
		constexpr float maxPitch = 89.0f; // Maximum allowed pitch angle in degrees
		constexpr float minPitch = -89.0f; // Minimum allowed pitch angle in degrees

		static float yaw = 0.0f; // Initial yaw angle
		static float pitch = 0.0f; // Initial pitch angle

		// Update yaw angle (horizontal movement)
		yaw += yawChange;

		// Update pitch angle (vertical movement) with limits
		pitch += pitchChange;
		pitch = fmaxf(fminf(pitch, maxPitch), minPitch);

		// Call Look function with the updated yaw and pitch angles
		Look(yaw, pitch);
	}

	float2 prevMousePos = float2();
	float aspect = static_cast<float>(SCRWIDTH) / static_cast<float>(SCRHEIGHT);
	float3 camPos = {2.5,1,0.5}, camTarget = {1.5f,0.8f,0.5f};
	float camSpeed = 1, camLookSpeed = 10.f;
	float3 topLeft;
	float3 topRight;
	float3 bottomLeft;

	int numFramesToAccumulate = 8;
	bool bAccumulate = true;

	float focalLength = 2.3f;
	float apertureRadius = 0.f;
};

}