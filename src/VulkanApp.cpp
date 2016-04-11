#include "VulkanApp.h"
#include "VulkanDebug.h"

#define VERTEX_BUFFER_BIND_ID 0

VulkanApp::VulkanApp() : VulkanBase()
{
	
}

VulkanApp::~VulkanApp()
{
	// Cleanup vertex buffer
	vkDestroyBuffer(device, vertices.buffer, nullptr);
	vkFreeMemory(device, vertices.memory, nullptr);

	// Cleanup index buffer
	vkDestroyBuffer(device, indices.buffer, nullptr);
	vkFreeMemory(device, indices.memory, nullptr);

	// Cleanup uniform buffer
	vkDestroyBuffer(device, uniformBuffer.buffer, nullptr);
	vkFreeMemory(device, uniformBuffer.memory, nullptr);

	// Cleanup descriptor set layout and pipeline layout
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

void VulkanApp::Prepare()
{
	VulkanBase::Prepare();

	PrepareVertices();
	PrepareUniformBuffers();
	
	SetupDescriptorSetLayout();
	//PreparePipelines();				// TODO
	SetupDescriptorPool();
	SetupDescriptorSet();

	// This records all the rendering commands to a command buffer
	// The command buffer will be sent to VkQueueSubmit in Draw() but this code only runs once
	RecordRenderingCommandBuffer();

	// Stuff unclear: swapchain, framebuffer, renderpass
}

void VulkanApp::PrepareVertices()
{
	VkMemoryRequirements memoryRequirments;
	VkMemoryAllocateInfo memoryAllocation = {};
	memoryAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	void *data;				// Used for memcpy

	//
	// Create the vertex buffer
	//

	struct Vertex {
		float pos[3];		// VK_FORMAT_R32G32B32_SFLOAT
		float col[3];		// VK_FORMAT_R32G32B32_SFLOAT
	};

	// Vertices
	std::vector<Vertex> vertexBuffer = {
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	int vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.usage	= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.size	= vertexBufferSize;

	VulkanDebug::ErrorCheck( vkCreateBuffer(device, &vertexBufferInfo, nullptr, &vertices.buffer) );								// Create buffer
	vkGetBufferMemoryRequirements(device, vertices.buffer, &memoryRequirments);														// Get buffer size
	memoryAllocation.allocationSize = memoryRequirments.size;
	GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
	VulkanDebug::ErrorCheck( vkAllocateMemory(device, &memoryAllocation, nullptr, &vertices.memory) );								// Allocate device memory
	VulkanDebug::ErrorCheck( vkMapMemory(device, vertices.memory, 0, memoryAllocation.allocationSize, 0, &data) );					// Map device memory so the host can access it through data
	memcpy(data, vertexBuffer.data(), vertexBufferSize);																			// Copy buffer data to the mapped data pointer
	vkUnmapMemory(device, vertices.memory);																							// Unmap memory
	VulkanDebug::ErrorCheck( vkBindBufferMemory(device, vertices.buffer, vertices.memory, 0) );										// Bind the buffer to the allocated device memory

	//
	// Create the index buffer
	//

	// Indices
	std::vector<uint32_t> indexBuffer	= { 0, 1, 2 };
	uint32_t indexBufferSize			= indexBuffer.size() * sizeof(uint32_t);
	indices.count						= indexBuffer.size();

	VkBufferCreateInfo indexBufferInfo = {};
	indexBufferInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferInfo.usage	= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.size	= indexBufferSize;

	memset(&indices, 0, sizeof(indices));
	VulkanDebug::ErrorCheck( vkCreateBuffer(device, &indexBufferInfo, nullptr, &indices.buffer) );									// Create buffer
	vkGetBufferMemoryRequirements(device, indices.buffer, &memoryRequirments);														// Get buffer size
	memoryAllocation.allocationSize = memoryRequirments.size;																		
	GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocation.memoryTypeIndex);		// Get memory type
	VulkanDebug::ErrorCheck( vkAllocateMemory(device, &memoryAllocation, nullptr, &indices.memory) );								// Allocate device memory
	VulkanDebug::ErrorCheck( vkMapMemory(device, indices.memory, 0, memoryAllocation.allocationSize, 0, &data) );					// Map device memory so the host can access it through data
	memcpy(data, indexBuffer.data(), indexBufferSize);																				// Copy buffer data to the mapped data pointer
	vkUnmapMemory(device, indices.memory);																							// Unmap memory
	VulkanDebug::ErrorCheck( vkBindBufferMemory(device, indices.buffer, indices.memory, 0) );										// Bind the buffer to the allocated device memory
	indices.count = indexBuffer.size();

	//
	// Vertex binding
	// 

	// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0].binding		= VERTEX_BUFFER_BIND_ID;				// Bind to ID 0, this information will be used by the shader
	vertices.bindingDescriptions[0].stride		= sizeof(Vertex);						// Size of each vertex
	vertices.bindingDescriptions[0].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;		

	// Now we also need to tell Vulkan about the memory layout for each attribute
	// Our vertex structure looks like
	//	struct Vertex {
	//		float pos[3];	= 3 * float = 3 * 32 bits = VK_FORMAT_R32G32B32_SFLOAT
	//		float col[3];	= 3 * float = 3 * 32 bits = VK_FORMAT_R32G32B32_SFLOAT
	//	};

	// 2 attributes, 1 for position and 1 for color
	vertices.attributeDescriptions.resize(2);	

	// Location 0 : Position
	vertices.attributeDescriptions[0].binding	= VERTEX_BUFFER_BIND_ID;
	vertices.attributeDescriptions[0].location	= 0;									// Location 0 (will be used in the shader)
	vertices.attributeDescriptions[0].format	= VK_FORMAT_R32G32B32_SFLOAT;
	vertices.attributeDescriptions[0].offset	= 0;									// First attribute can start at offset 0
	vertices.attributeDescriptions[0].binding	= 0;

	// Location 1 : Color
	vertices.attributeDescriptions[1].binding	= VERTEX_BUFFER_BIND_ID;
	vertices.attributeDescriptions[1].location	= 1;									// Location 1 (will be used in the shader)
	vertices.attributeDescriptions[1].format	= VK_FORMAT_R32G32B32_SFLOAT;
	vertices.attributeDescriptions[1].offset	= sizeof(float) * 3;					// Second attribute needs to start with offset = sizeof(attribute 1)
	vertices.attributeDescriptions[1].binding	= 0;

	// Neither the bindingDescriptions or the attributeDescriptions is used directly
	// When creating a graphics pipeline a VkPipelineVertexInputStateCreateInfo structure is sent as an argument and this structure
	// contains the VkVertexInputBindingDescription and VkVertexInputAttributeDescription
	// The last thing to do is to assign the binding and attribute descriptions

	vertices.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertices.inputState.pNext = NULL;
	vertices.inputState.vertexBindingDescriptionCount	= vertices.bindingDescriptions.size();
	vertices.inputState.pVertexBindingDescriptions		= vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
	vertices.inputState.pVertexAttributeDescriptions	= vertices.attributeDescriptions.data();
}

void VulkanApp::PrepareUniformBuffers()
{
	// Create the uniform buffer
	// It's the same process as creating any buffer, except that the VkBufferCreateInfo.usage bit is different (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	VkBufferCreateInfo createInfo = {};
	VkMemoryAllocateInfo allocInfo = {};
	VkMemoryRequirements memoryRequirments = {};

	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	= 0;								// Gets assigned with vkGetBufferMemoryRequirements
	allocInfo.memoryTypeIndex	= 0;

	createInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size		= sizeof(uniformData);						// 3x glm::mat4
	createInfo.usage	= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VulkanDebug::ErrorCheck(vkCreateBuffer(device, &createInfo, nullptr, &uniformBuffer.buffer));
	vkGetBufferMemoryRequirements(device, uniformBuffer.buffer, &memoryRequirments);
	allocInfo.allocationSize = memoryRequirments.size;
	GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);
	VulkanDebug::ErrorCheck(vkAllocateMemory(device, &allocInfo, nullptr, &uniformBuffer.memory));
	VulkanDebug::ErrorCheck(vkBindBufferMemory(device, uniformBuffer.buffer, uniformBuffer.memory, 0));
	
	// uniformBuffer.buffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
	// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
	uniformBuffer.descriptor.buffer = uniformBuffer.buffer;
	uniformBuffer.descriptor.offset = 0;
	uniformBuffer.descriptor.range = sizeof(uniformData);		// 3x glm::mat4

	// This is where the data gets transfered to device memory w/ vkMapMemory/vkUnmapMemory and memcpy
	UpdateUniformBuffers();
}

void VulkanApp::SetupDescriptorSetLayout()
{
	// Only one binding used for the uniform buffer containing the matrices (Binding: 0)
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding				= 0;
	layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount		= 1;
	layoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;				// Will be used by the vertex shader
	layoutBinding.pImmutableSamplers	= NULL;

	// One more binding would be used for a texture (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER and VK_SHADER_STAGE_FRAGMENT_BIT)

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings	= &layoutBinding;

	VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout));

	// Create the pipeline layout that will use the descriptor set layout
	// The pipeline layout is used later when creating the pipeline
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext				= NULL;
	pPipelineLayoutCreateInfo.setLayoutCount	= 1;
	pPipelineLayoutCreateInfo.pSetLayouts		= &descriptorSetLayout;

	VulkanDebug::ErrorCheck(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanApp::SetupDescriptorPool()
{
	// We need to tell the API the number of max. requested descriptors per type
	// Only one descriptor type (uniform buffer) used
	// More needed if images are used etc.
	VkDescriptorPoolSize typeCounts[1];
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 1;		

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = typeCounts;
	createInfo.maxSets = 1;

	VulkanDebug::ErrorCheck(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));
}

void VulkanApp::SetupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool		= descriptorPool;
	allocInfo.descriptorSetCount	= 1;
	allocInfo.pSetLayouts			= &descriptorSetLayout;

	VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

	// Binding 0 : Uniform buffer
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet			= descriptorSet;
	writeDescriptorSet.descriptorCount	= 1;
	writeDescriptorSet.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pBufferInfo		= &uniformBuffer.descriptor;
	// Binds this uniform buffer to binding point 0
	writeDescriptorSet.dstBinding		= 0;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

void VulkanApp::PreparePipelines()
{
	// The pipeline consists of many stages, where each stage can have different states
	// Creating a pipeline is simply defining the state for every stage
	// ...

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout				= pipelineLayout;
	pipelineCreateInfo.renderPass			= renderPass;
	pipelineCreateInfo.pVertexInputState	= &vertices.inputState;
	// TODO... a lot...


	// Create the pipeline
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
}

// Call this every time any uniform buffer should be updated (view changes etc.)
void VulkanApp::UpdateUniformBuffers()
{
	uniformData.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 256.0f);

	float zoom = 0;
	uniformData.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

	uniformData.modelMatrix = glm::mat4();	// Identity matrix, I think?

	// Map uniform buffer and update it
	uint8_t *data;
	VkResult err = vkMapMemory(device, uniformBuffer.memory, 0, sizeof(uniformData), 0, (void **)&data);
	assert(!err);
	memcpy(data, &uniformData, sizeof(uniformData));
	vkUnmapMemory(device, uniformBuffer.memory);
	assert(!err);
}

void VulkanApp::RecordRenderingCommandBuffer()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearColorValue clear_color = {
		{ 1.0f, 0.8f, 0.4f, 0.0f }
	};

	VkImageSubresourceRange image_subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
		0,                                            // uint32_t                               baseMipLevel
		1,                                            // uint32_t                               levelCount
		0,                                            // uint32_t                               baseArrayLayer
		1                                             // uint32_t                               layerCount
	};

	for (int i = 0; i < renderingCommandBuffers.size(); i++)
	{
		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(renderingCommandBuffers[i], &beginInfo));

		vkCmdClearColorImage(renderingCommandBuffers[i], swapChain.images[i], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_subresource_range);	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(renderingCommandBuffers[i]));
	}
	
}

void VulkanApp::Render()
{
	//if (!prepared)
	//	return;

	vkDeviceWaitIdle(device);
	Draw();
	vkDeviceWaitIdle(device);
}

void VulkanApp::Draw()
{
	// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
	// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
	// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 

	// Acquire the next image in the swap chain
	VulkanDebug::ErrorCheck(swapChain.acquireNextImage(presentComplete, &currentBuffer));
	
	//
	// Transition image format to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//

	SubmitPrePresentMemoryBarrier(swapChain.buffers[currentBuffer].image);

	//
	// Do rendering
	//

	// Submit the recorded draw command buffer to the queue
	VkSubmitInfo submitInfo = {};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &renderingCommandBuffers[currentBuffer];		// Draw commands for the current command buffer
	submitInfo.waitSemaphoreCount	= 1;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores		= &presentComplete;							// Waits for swapChain.acquireNextImage to complete
	submitInfo.pSignalSemaphores	= &renderComplete;							// swapChain.queuePresent will wait for this submit to complete
	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	submitInfo.pWaitDstStageMask	= &stageFlags;

	VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	//
	// Transition image format to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	//

	SubmitPostPresentMemoryBarrier(swapChain.buffers[currentBuffer].image);

	// Present the image
	VulkanDebug::ErrorCheck(swapChain.queuePresent(queue, currentBuffer, renderComplete));
}