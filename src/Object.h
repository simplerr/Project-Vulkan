#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
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
		void SetColor(glm::vec3 color);

		void SetPipeline(VkPipeline pipeline);


		StaticModel* GetModel();
		glm::vec3 GetPosition();
		glm::vec3 GetRotation();
		glm::vec3 GetScale();
		glm::vec3 GetColor();
		glm::mat4 GetWorldMatrix();

		VkPipeline GetPipeline();
	private:
		StaticModel* mModel;
		glm::vec3 mPosition;
		glm::vec3 mRotation;
		glm::vec3 mScale;
		glm::vec3 mColor;

		VkPipeline mPipeline;		// Points to a pipeline object stored somewhere else
	};
}	// VulkanLib namespace