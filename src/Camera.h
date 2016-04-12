#pragma once

#include <windows.h>
#include <glm/glm.hpp>

class Camera
{
public:
	Camera();

	void HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	glm::vec3 GetDirection();
	void UpdatePitchYaw();

	glm::mat4 GetViewMatrix();
private:
	glm::mat4 viewMatrix;

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 direction;
	glm::vec3 up;
	glm::vec3 right;

	float yaw, pitch;

	float sensitivity = 0.01f;
	float speed = 1.0f;

	int lastX, lastY;
};