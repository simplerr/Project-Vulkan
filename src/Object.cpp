#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(glm::vec3 position)
{
	SetPosition(position);
	SetRotation(glm::vec3(0, 0, 0));
	SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
	SetModel(nullptr);
	SetPipeline(VK_NULL_HANDLE);		// Must be assigned later
}

Object::~Object()
{
}

void Object::SetModel(StaticModel * model)
{
	mModel = model;
}

void Object::SetPosition(glm::vec3 position)
{
	mPosition = position;
}

void Object::SetRotation(glm::vec3 rotation)
{
	mRotation = rotation;
}

void Object::SetScale(glm::vec3 scale)
{
	mScale = scale;
}

void Object::SetPipeline(VkPipeline pipeline)
{
	mPipeline = pipeline;
}

StaticModel * Object::GetModel()
{
	return mModel;
}

glm::vec3 Object::GetPosition()
{
	return mPosition;
}

glm::vec3 Object::GetRotation()
{
	return mRotation;
}

glm::vec3 Object::GetScale()
{
	return mScale;
}

glm::mat4 Object::GetWorldMatrix()
{
	glm::mat4 world;

	world = glm::translate(world, mPosition);
	world = glm::rotate(world, glm::radians(mRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	world = glm::rotate(world, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	world = glm::rotate(world, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	world = glm::scale(world, mScale);

	return world;
}

VkPipeline Object::GetPipeline()
{
	return mPipeline;
}
