#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>

namespace VulkanLib
{
	Object::Object(vec3 position)
	{
		SetPosition(position);
		SetRotation(vec3(0, 0, 0));
		SetScale(vec3(1.0f, 1.0f, 1.0f));
		SetColor(vec3(1.0f, 1.0f, 1.0f));
		SetModel(nullptr);
		SetPipeline(VK_NULL_HANDLE);		// Must be assigned later
		SetId(0);
	}

	Object::~Object()
	{
	}

	void Object::SetModel(StaticModel * model)
	{
		mModel = model;
	}

	void Object::SetPosition(vec3 position)
	{
		mPosition = position;
		RebuildWorldMatrix();
	}

	void Object::SetRotation(vec3 rotation)
	{
		mRotation = rotation;
		RebuildWorldMatrix();
	}

	void Object::SetScale(vec3 scale)
	{
		mScale = scale;
		RebuildWorldMatrix();
	}

	void Object::SetColor(vec3 color)
	{
		mColor = color;
	}

	void Object::SetId(int id)
	{
		mId = id;
	}

	void Object::AddRotation(float x, float y, float z)
	{
		mRotation += vec3(x, y, z);
		RebuildWorldMatrix();
	}

	void Object::SetPipeline(VkPipeline pipeline)
	{
		mPipeline = pipeline;
	}

	StaticModel * Object::GetModel()
	{
		return mModel;
	}

	vec3 Object::GetPosition()
	{
		return mPosition;
	}

	vec3 Object::GetRotation()
	{
		return mRotation;
	}

	vec3 Object::GetScale()
	{
		return mScale;
	}

	vec3 Object::GetColor()
	{
		return mColor;
	}

	mat4 Object::GetWorldMatrix()
	{
		return mWorld;
	}

	int Object::GetId()
	{
		return mId;
	}

	VkPipeline Object::GetPipeline()
	{
		return mPipeline;
	}
	void Object::RebuildWorldMatrix()
	{
		mat4 world;

		world = glm::translate(world, mPosition);
		world = glm::rotate(world, glm::radians(mRotation.x), vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(mRotation.y), vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(mRotation.z), vec3(0.0f, 0.0f, 1.0f));
		world = glm::scale(world, mScale);

		mWorld = world;
	}
}	// VulkanLib namespace