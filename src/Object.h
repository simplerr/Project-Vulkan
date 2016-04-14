#pragma once

#include <glm/glm.hpp>

class StaticModel;

class Object
{
public:
	Object(glm::vec3 position);
	~Object();

	void SetModel(StaticModel* model);
	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);

	StaticModel* GetModel();
	glm::vec3 GetPosition();
	glm::vec3 GetRotation();
	glm::vec3 GetScale();
	glm::mat4 GetWorldMatrix();
private:
	StaticModel* mModel;
	glm::vec3 mPosition;
	glm::vec3 mRotation;
	glm::vec3 mScale;
};