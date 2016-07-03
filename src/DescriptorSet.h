#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanDebug.h"
#include "VulkanHelpers.h"

namespace VulkanLib
{
	class DescriptorSet
	{
	public:
		void Cleanup(VkDevice device);

		void AddLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
		void CreateLayout(VkDevice device);

		void AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool);
		void UpdateDescriptorSets(VkDevice device);

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
		// Add more binding function when needed...	
			
		// Public for ease of use
		VkDescriptorSetLayout setLayout;
		VkDescriptorSet descriptorSet;
	private:
		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
		std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
	};
}
