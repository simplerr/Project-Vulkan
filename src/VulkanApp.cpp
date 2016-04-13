#include <array>

#include "VulkanApp.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"

#define VERTEX_BUFFER_BIND_ID 0

VulkanApp::VulkanApp() : VulkanBase()
{
	camera = new Camera(glm::vec3(100, 100, 100), 60.0f, (float)windowWidth / (float)windowHeight, 0.1f, 2560.0f);
	camera->LookAt(glm::vec3(0, 0, 0));
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

	vkDestroyPipeline(device, pipeline, nullptr);

	// Has to cleanup for all the models TODO: List with many models need to be looped
	vkDestroyBuffer(device, testModel->vertices.buffer, nullptr);
	vkFreeMemory(device, testModel->vertices.memory, nullptr);
	vkDestroyBuffer(device, testModel->indices.buffer, nullptr);
	vkFreeMemory(device, testModel->indices.memory, nullptr);

	textureLoader->destroyTexture(testModel->texture);

	delete testModel;
	delete camera;
}

void VulkanApp::Prepare()
{
	VulkanBase::Prepare();

	PrepareVertices();
	PrepareUniformBuffers();
	LoadModels();						// Custom
	SetupVertexDescriptions();			// Custom
	SetupDescriptorSetLayout();
	PreparePipelines();				
	SetupDescriptorPool();
	SetupDescriptorSet();

	// This records all the rendering commands to a command buffer
	// The command buffer will be sent to VkQueueSubmit in Draw() but this code only runs once
	RecordRenderingCommandBuffer();

	prepared = true;

	// Stuff unclear: swapchain, framebuffer, renderpass
}

void VulkanApp::LoadModels()
{
	//testModel = modelLoader.LoadModel("models/voyager/voyager.obj");// voyager / voyager.obj");
	testModel = modelLoader.LoadModel("models/teapot.3ds");
	//testModel = modelLoader.LoadModel("models/samplescene.obj");

	testModel->BuildBuffers(this);

	textureLoader->loadTexture("models/voyager/voyager.ktx", VK_FORMAT_BC3_UNORM_BLOCK, &testModel->texture);

	// Generate some positions
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			modelPositions.push_back(glm::vec3(i * 100, 0, j * 100));
		}
	}


	// TODO: Needs setup the binding descriptions
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
		float col[4];		// VK_FORMAT_R32G32B32_SFLOAT
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
	createInfo.size		= sizeof(uniformData);						// 3x glm::mat4 NOTE: Not any more!!
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
	std::vector<VkDescriptorSetLayoutBinding> layoutBinding = {
		// Binding 0: Uniform buffer
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,				// Will be used by the vertex shader
			NULL
		},
		//  Binding 1: Image sampler
		{
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,			// Will be used by the vertex shader
			NULL
		}
	};

	// One more binding would be used for a texture (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER and VK_SHADER_STAGE_FRAGMENT_BIT)

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = layoutBinding.size();
	createInfo.pBindings	= layoutBinding.data();

	VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout));

	// Create the pipeline layout that will use the descriptor set layout
	// The pipeline layout is used later when creating the pipeline
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext				= NULL;
	pPipelineLayoutCreateInfo.setLayoutCount	= 1;
	pPipelineLayoutCreateInfo.pSetLayouts		= &descriptorSetLayout;

	// Note: New!!
	// Add push constants for the MVP matrix
	VkPushConstantRange pushConstantRanges = {};
	pushConstantRanges.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRanges.offset = 0;
	pushConstantRanges.size = sizeof(glm::mat4);

	pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRanges;

	VulkanDebug::ErrorCheck(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanApp::SetupDescriptorPool()
{
	// We need to tell the API the number of max. requested descriptors per type
	// Only one descriptor type (uniform buffer) used
	// More needed if images are used etc.
	VkDescriptorPoolSize typeCounts[2];
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// Uniform buffers
	typeCounts[0].descriptorCount = 1;		

	typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;		// Image sampler
	typeCounts[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 2;
	createInfo.pPoolSizes = typeCounts;
	createInfo.maxSets = 2;

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

	VkDescriptorImageInfo texDescriptor = {};
	texDescriptor.sampler = testModel->texture.sampler;				// NOTE: TODO: This feels really bad, not scalable with more objects at all, fix!!! LoadModel() must run before this!!
	texDescriptor.imageView = testModel->texture.view;
	texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	// Binding 0 : Uniform buffer
	std::vector<VkWriteDescriptorSet> writeDescriptorSet(2);
	writeDescriptorSet[0].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[0].dstSet			= descriptorSet;
	writeDescriptorSet[0].descriptorCount	= 1;
	writeDescriptorSet[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[0].pBufferInfo		= &uniformBuffer.descriptor;
	writeDescriptorSet[0].dstBinding		= 0;				// Binds this uniform buffer to binding point 0

	//  Binding 1: Image sampler
	writeDescriptorSet[1].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[1].dstSet			= descriptorSet;
	writeDescriptorSet[1].descriptorCount	= 1;
	writeDescriptorSet[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet[1].pImageInfo		= &texDescriptor;
	writeDescriptorSet[1].dstBinding		= 1;				// Binds the image sampler to binding point 1

	vkUpdateDescriptorSets(device, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, NULL);
}

void VulkanApp::PreparePipelines()
{
	// The pipeline consists of many stages, where each stage can have different states
	// Creating a pipeline is simply defining the state for every stage (and some more...)
	// ...

	// Input assembly state
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;

	// Color blend state
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;			// Blending disabled
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	
	// Dynamic state for the viewport so the pipeline don't have to be recreated when resizing the window
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = dynamicStateEnables.size();

	// Depth and stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.stencilTestEnable = VK_FALSE;			// Stencil disabled
	depthStencilState.front = depthStencilState.back;

	// Multi sampling state
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pSampleMask = NULL;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		// Multi sampling not used

	// Load shader
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = LoadShader("shaders/mesh/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/mesh/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Assign all the states to the pipeline
	// The states will be static and can't be changed after the pipeline is created
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.pVertexInputState = &vertexDescriptions.inputState;		// From base - &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();

	// Create the pipeline
	VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

// Call this every time any uniform buffer should be updated (view changes etc.)
void VulkanApp::UpdateUniformBuffers()
{
	uniformData.projectionMatrix = camera->GetProjection(); // glm::perspective(glm::radians(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 256.0f);

	float zoom = -8;
	glm::mat4 viewMatrix = camera->GetView(); //camera->GetViewMatrix();// glm::mat4();
	//viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, zoom));
	//uniformData.modelMatrix = glm::mat4();	// Identity matrix, I think?
	//uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 rotation(180, 0, 0);

	uniformData.modelMatrix = glm::mat4();
	uniformData.modelMatrix = viewMatrix * glm::translate(uniformData.modelMatrix, modelPos);
	uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// Map uniform buffer and update it
	uint8_t *data;
	VulkanDebug::ErrorCheck(vkMapMemory(device, uniformBuffer.memory, 0, sizeof(uniformData), 0, (void **)&data));
	memcpy(data, &uniformData, sizeof(uniformData));
	vkUnmapMemory(device, uniformBuffer.memory);
}

void VulkanApp::SetupVertexDescriptions()
{
	// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
	vertexDescriptions.bindingDescriptions.resize(1);
	vertexDescriptions.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;				// Bind to ID 0, this information will be used by the shader
	vertexDescriptions.bindingDescriptions[0].stride = sizeof(Vertex);						// Size of each vertex
	vertexDescriptions.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// We need to tell Vulkan about the memory layout for each attribute
	// 5 attributes: position, normal, texture coordinates, tangent and color
	// See Vertex struct
	vertexDescriptions.attributeDescriptions.resize(5);

	// Location 0 : Position
	vertexDescriptions.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	vertexDescriptions.attributeDescriptions[0].location = 0;								// Location 0 (will be used in the shader)
	vertexDescriptions.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexDescriptions.attributeDescriptions[0].offset = 0;									// First attribute can start at offset 0
	vertexDescriptions.attributeDescriptions[0].binding = 0;

	// Location 1 : Color
	vertexDescriptions.attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
	vertexDescriptions.attributeDescriptions[1].location = 1;								// Location 1 (will be used in the shader)
	vertexDescriptions.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexDescriptions.attributeDescriptions[1].offset = sizeof(float) * 3;					// Second attribute needs to start with offset = sizeof(attribute 1)
	vertexDescriptions.attributeDescriptions[1].binding = 0;

	// Location 2 : Normal
	vertexDescriptions.attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
	vertexDescriptions.attributeDescriptions[2].location = 2;								
	vertexDescriptions.attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexDescriptions.attributeDescriptions[2].offset = sizeof(float) * 6;					
	vertexDescriptions.attributeDescriptions[2].binding = 0;

	// Location 3 : Texture
	vertexDescriptions.attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
	vertexDescriptions.attributeDescriptions[3].location = 3;
	vertexDescriptions.attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	vertexDescriptions.attributeDescriptions[3].offset = sizeof(float) * 9;
	vertexDescriptions.attributeDescriptions[3].binding = 0;

	// Location 4 : Tangent
	vertexDescriptions.attributeDescriptions[4].binding = VERTEX_BUFFER_BIND_ID;
	vertexDescriptions.attributeDescriptions[4].location = 4;
	vertexDescriptions.attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexDescriptions.attributeDescriptions[4].offset = sizeof(float) * 11;
	vertexDescriptions.attributeDescriptions[4].binding = 0;

	// Neither the bindingDescriptions or the attributeDescriptions is used directly
	// When creating a graphics pipeline a VkPipelineVertexInputStateCreateInfo structure is sent as an argument and this structure
	// contains the VkVertexInputBindingDescription and VkVertexInputAttributeDescription
	// The last thing to do is to assign the binding and attribute descriptions
	vertexDescriptions.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexDescriptions.inputState.pNext = NULL;
	vertexDescriptions.inputState.vertexBindingDescriptionCount = vertexDescriptions.bindingDescriptions.size();
	vertexDescriptions.inputState.pVertexBindingDescriptions = vertexDescriptions.bindingDescriptions.data();
	vertexDescriptions.inputState.vertexAttributeDescriptionCount = vertexDescriptions.attributeDescriptions.size();
	vertexDescriptions.inputState.pVertexAttributeDescriptions = vertexDescriptions.attributeDescriptions.data();
}

void VulkanApp::RecordRenderingCommandBuffer()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { 1.0f, 0.8f, 0.4f, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = windowWidth;
	renderPassBeginInfo.renderArea.extent.height = windowHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int i = 0; i < renderingCommandBuffers.size(); i++)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		// Begin command buffer recording & the render pass
		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(renderingCommandBuffers[i], &beginInfo));
		vkCmdBeginRenderPass(renderingCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//vkCmdClearColorImage(renderingCommandBuffers[i], swapChain.images[i], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_subresource_range);	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = (float)windowWidth;
		viewport.height = (float)windowHeight;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(renderingCommandBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = windowWidth;
		scissor.extent.height = windowHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(renderingCommandBuffers[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(renderingCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		// Bind the rendering pipeline (including the shaders)
		vkCmdBindPipeline(renderingCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// Bind triangle vertices
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(renderingCommandBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &testModel->vertices.buffer, offsets);		// NOTE: testModel->vertices.buffer for testing

		// Bind triangle indices
		vkCmdBindIndexBuffer(renderingCommandBuffers[i], testModel->indices.buffer, 0, VK_INDEX_TYPE_UINT32);					// NOTE: testModel->indices.buffer for testing
		
		//
		// Testing push constant rendering with different matrices
		//
		for (auto pos : modelPositions)
		{
			glm::vec3 rotation(180, 0, 0);

			// Push the MVP constant
			glm::mat4 model = glm::translate(glm::mat4(), pos) * glm::rotate(glm::mat4(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 mvp = camera->GetProjection() * camera->GetView() * model;
			int siss = sizeof(mvp);
			vkCmdPushConstants(renderingCommandBuffers[i], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

			// Draw indexed triangle																													
			vkCmdDrawIndexed(renderingCommandBuffers[i], testModel->GetNumIndices(), 1, 0, 0, 0);
		}

		// End command buffer recording & the render pass
		vkCmdEndRenderPass(renderingCommandBuffers[i]);
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(renderingCommandBuffers[i]));
	}	
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

	// NOTE: Testing
	RecordRenderingCommandBuffer();

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

void VulkanApp::Render()
{
	//if (!prepared)
	//	return;

	vkDeviceWaitIdle(device);

	camera->Update();

	// NOTE: TODO: TESTING
	if (prepared)
		UpdateUniformBuffers();

	Draw();

	vkDeviceWaitIdle(device);
}

void VulkanApp::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Default message handling
	VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);

	// Let the camera handle user input
	camera->HandleMessages(hwnd, msg, wParam, lParam);

	// Testing
	if(msg == WM_KEYDOWN && wParam == 'R')
	{
		modelPos = glm::vec3(0, -50, 0);
	}
	else if (msg == WM_KEYDOWN && wParam == 'T')
	{
		modelPos = glm::vec3(0, 0, 0);
	}
}