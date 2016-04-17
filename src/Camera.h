#pragma once

#include "Platform.h"
#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 position, float fieldOfView, float aspectRatio, float nearPlane, float farPlane);

	void Update();

#if defined(_WIN32)
	void HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

	glm::vec3 GetDirection();

	// New
	glm::mat4 GetOrientation();
	glm::mat4 GetView();
	glm::mat4 GetProjection();
	glm::mat4 GetMatrix();
	glm::vec3 GetRight();
	glm::vec3 GetPosition();
	void AddOrientation(float yaw, float pitch);
	void LookAt(glm::vec3 target);
	void CapAngles();

private:
	glm::vec3 position;
	float pitch;	// Vertical angle
	float yaw;		// Horizontal angle
	float fov;
	float nearPlane;
	float farPlane;
	float aspectRatio;

	float sensitivity = 0.2f;
	float speed = 2.5f;

	int lastX, lastY;
};