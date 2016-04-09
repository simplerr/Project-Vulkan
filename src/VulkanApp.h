#pragma once

#include "VulkanBase.h"

class VulkanApp : public VulkanBase
{
public:
	VulkanApp();
	~VulkanApp();

	void Prepare();

	void PrepareVertices();


	// Wraps everything that has to do with the vertices
	// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
	// inputState is the pVertexInputState when creating the graphics pipeline
	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	struct {
		int count;
		VkBuffer buffer;
		VkDeviceMemory memory;
	} indices;
};
