#include "Camera.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"

Camera::Camera()
{
	position = glm::vec3(70, 70, 70);
	target = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);

	lastX = lastY = -1;
}

void Camera::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
	{
		if (wParam == 'W') {
			position += speed * GetDirection();
			target += speed * GetDirection();
		}
		if (wParam == 'S') {
			position -= speed * GetDirection();
			target -= speed * GetDirection();
		}

		viewMatrix = glm::lookAt(position, target, up);

		break;
	}
	case WM_LBUTTONDOWN:
	{
		int a = 9;
		break;
	}
	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (lastX == -1 && lastY == -1) {
			lastX = x;
			lastY = y;
			break;
		}

		float dx = x - lastX;
		float dy = y - lastY;

		yaw += dx * sensitivity;
		pitch += dy * sensitivity;

		// Limit to PI/2 radians.
		if (pitch > 0)
			pitch = glm::min(pitch, 1.54f);
		else
			pitch = glm::max(pitch, -1.54f);

		// Calculate the new direction.
		
		float r = cosf(pitch);
		glm::vec3 direction(r * sinf(yaw), sinf(pitch), r * cosf(yaw));
		target = position + direction;

		viewMatrix = glm::lookAt(position, target, up); // glm::vec3(0, 0, 0)

		lastX = x;
		lastY = y;

		break;
	}
	default:
		break;
	}
}

glm::vec3 Camera::GetDirection()
{
	return normalize(target - position);
}

void Camera::UpdatePitchYaw()
{
	glm::vec3 dir = GetDirection();
	pitch = asinf(dir.y);
	yaw = atan2f(dir.x, dir.z);
}

glm::mat4 Camera::GetViewMatrix()
{
	return viewMatrix;
}
