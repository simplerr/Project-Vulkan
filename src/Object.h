#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

using namespace glm;

namespace VulkanLib
{
	class StaticModel;

	class Object
	{
	public:
		Object(vec3 position);
		~Object();

		void SetModel(StaticModel* model);
		void SetPosition(vec3 position);
		void SetRotation(vec3 rotation);
		void SetScale(vec3 scale);
		void SetColor(vec3 color);
		void SetId(int id);

		void AddRotation(float x, float y, float z);

		void SetPipeline(VkPipeline pipeline);


		StaticModel* GetModel();
		vec3 GetPosition();
		vec3 GetRotation();
		vec3 GetScale();
		vec3 GetColor();
		mat4 GetWorldMatrix();
		int GetId();

		VkPipeline GetPipeline();
	private:
		StaticModel* mModel;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
		vec3 mColor;
		int mId; 

		VkPipeline mPipeline;		// Points to a pipeline object stored somewhere else
	};
}	// VulkanLib namespace