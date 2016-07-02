#pragma once
#include <vulkan/vulkan.h>
#include "VulkanBase.h"

namespace VulkanLib
{
	class UniformBuffer
	{
	public:
		void Cleanup(VkDevice device)
		{
			// Cleanup uniform buffer
			vkDestroyBuffer(device, mBuffer, nullptr);
			vkFreeMemory(device, mMemory, nullptr);
		}

		void CreateBuffer(VulkanBase* vulkanBase)
		{
			vulkanBase->CreateBuffer(
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
				GetSize(),	// Virtual function
				nullptr, 
				&mBuffer, 
				&mMemory);

			// mBuffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
			// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
			mDescriptor.buffer = mBuffer;
			mDescriptor.range = GetSize();
			mDescriptor.offset = 0;
		}

		// This is where the data gets transfered to device memory w/ vkMapMemory,vkUnmapMemory and memcpy
		virtual void UpdateMemory(VkDevice device) = 0;

		virtual int GetSize() = 0;

		VkDescriptorBufferInfo GetDescriptor() { return mDescriptor; }

	protected:
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		VkDescriptorBufferInfo mDescriptor;
	};

	
}
