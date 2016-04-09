#include "VulkanApp.h"
#include "VulkanDebug.h"

#define VERTEX_BUFFER_BIND_ID 0

VulkanApp::VulkanApp() : VulkanBase()
{
	
}

VulkanApp::~VulkanApp()
{

}

void VulkanApp::Prepare()
{
	VulkanBase::Prepare();

	// Pipeline
	// Uniform buffers
	// Vertex buffers 
	// Descriptor sets


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
