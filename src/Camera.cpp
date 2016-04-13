#include "Camera.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"

Camera::Camera()
{
	position = glm::vec3(7, 7, 7);

	yaw = pitch = 0.0f;
	fov = 60.0f;
	nearPlane = 0.1f;
	farPlane = 256.0f;
	aspectRatio = 4.0f / 3.0f;

	lastX = lastY = -1;
}

Camera::Camera(glm::vec3 position, float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
{
	this->position = position;
	this->fov = fieldOfView;
	this->aspectRatio = aspectRatio;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	yaw = pitch = 0.0f;
	lastX = lastY = -1;
}

void Camera::Update()
{
	if (GetAsyncKeyState('W')) {
		glm::vec3 dir = GetDirection();
		position += speed * dir;

	}
	if (GetAsyncKeyState('S')) {
		glm::vec3 dir = GetDirection();
		position -= speed * dir;

	}
	if (GetAsyncKeyState('A')) {
		glm::vec3 right = GetRight();
		position += speed * right;

	}
	if (GetAsyncKeyState('D')) {
		glm::vec3 right = GetRight();
		position -= speed * right;

	}
}

void Camera::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	
	case WM_LBUTTONDOWN:
	{
		lastX = LOWORD(lParam);
		lastY = HIWORD(lParam);
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (wParam & MK_LBUTTON)
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			if (lastX == -1 && lastY == -1) {
				lastX = x;
				lastY = y;
				break;
			}

			float dx = x - lastX;
			float dy = lastY - y;		// Other way around

			yaw += dx * sensitivity;
			pitch += dy * sensitivity;

			CapAngles();

			lastX = x;
			lastY = y;
		}

		break;
	}
	default:
		break;
	}
}

glm::vec3 Camera::GetDirection()
{
	/*float r = cosf(glm::radians(pitch));
	glm::vec3 direction(r * sinf(glm::radians(yaw)), sinf(glm::radians(pitch)), r * cosf(glm::radians(yaw)));
	direction = glm::normalize(direction);
	return direction;*/

	glm::vec4 forward = glm::inverse(GetOrientation()) * glm::vec4(0, 0, 1, 1);
	glm::vec3 f = forward;
	if (f.y != -1.0f)
		int a = 1;
	f = glm::normalize(f);
	return f;
}

glm::vec3 Camera::GetRight()
{
	glm::vec4 right = glm::inverse(GetOrientation()) * glm::vec4(1, 0, 0, 1);
	glm::vec3 r = right;
	r = glm::normalize(r);
	return r;
	//return glm::normalize(right);
}

glm::mat4 Camera::GetOrientation()
{
	glm::mat4 orientation;
	orientation = glm::rotate(orientation, glm::radians(pitch), glm::vec3(1, 0, 0));		// Pitch (vertical angle)
	orientation = glm::rotate(orientation, glm::radians(yaw), glm::vec3(0, 1, 0));		// Yaw (horizontal angle)
	return orientation;
}

void Camera::AddOrientation(float yaw, float pitch)
{
	this->yaw += yaw;
	this->pitch += pitch;
	CapAngles();
}

glm::mat4 Camera::GetView()
{
	return GetOrientation() * glm::translate(glm::mat4(), position);	
}

glm::mat4 Camera::GetProjection()
{
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

glm::mat4 Camera::GetMatrix()
{
	return GetProjection() * GetView();
}

void Camera::LookAt(glm::vec3 target)
{
	glm::vec3 dir = glm::normalize(target - position);
	pitch = glm::degrees(asinf(dir.y));
	yaw = -glm::degrees(atan2f(dir.x, dir.z));		// Note the - signs
}

void Camera::CapAngles()
{
	yaw = fmodf(yaw, 360.0f);
	
	if (yaw < 0.0f)
		yaw += 360.0f;

	if (pitch > 85.0f)
		pitch = 85.0;
	else if (pitch < -85.0)
		pitch = -85.0;
}
