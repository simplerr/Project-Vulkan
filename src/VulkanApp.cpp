#include <array>
#include <time.h>
#include <cstdlib>

#include "VulkanApp.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "Object.h"
#include "LoadTGA.h"

#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION true		// Debug validation layers toggle (affects performance a lot)

// Temporary defines to not rotate the sky sphere and terrain
#define OBJECT_ID_SKY 1
#define OBJECT_ID_TERRAIN 2
#define OBJECT_ID_PROP 3

namespace VulkanLib
{
	VulkanApp::VulkanApp() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
	}

	VulkanApp::~VulkanApp()
	{
		// Cleanup uniform buffer
		vkDestroyBuffer(mDevice, mUniformBuffer.buffer, nullptr);
		vkFreeMemory(mDevice, mUniformBuffer.memory, nullptr);

		// Cleanup descriptor set layout and pipeline layout
		vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

		vkDestroyPipeline(mDevice, mPipelines.textured, nullptr);
		vkDestroyPipeline(mDevice, mPipelines.colored, nullptr);
		vkDestroyPipeline(mDevice, mPipelines.starsphere, nullptr);

		// The model loader is responsible for cleaning up the model data
		mModelLoader.CleanupModels(mDevice);

		// Free the testing texture
		mTextureLoader->destroyTexture(mTestTexture);
		mTextureLoader->destroyTexture(mTerrainTexture);

		for (int i = 0; i < mObjects.size(); i++) {
			delete mObjects[i];
		}
	}

	void VulkanApp::Prepare()
	{
		VulkanBase::Prepare();


		SetupVertexDescriptions();			// Custom
		SetupDescriptorSetLayout();
		PreparePipelines();
		LoadModels();						// Custom
		PrepareUniformBuffers();
		SetupDescriptorPool();
		SetupDescriptorSet();
		PrepareCommandBuffers();
		//SetupTerrainDescriptorSet();		// Custom

		// This records all the rendering commands to a command buffer
		// The command buffer will be sent to VkQueueSubmit in Draw() but this code only runs once
		//RecordRenderingCommandBuffer();

		mPrepared = true;

		// Stuff unclear: swapchain, framebuffer, renderpass
	}

	void VulkanApp::PrepareCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		// Create the primary command buffer
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mPrimaryCommandBuffer));

		// Create the secondary command buffer
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mSecondaryCommandBuffer));
	}

	void VulkanApp::CompileShaders()
	{
		system("cd data/shaders/textured/ && generate-spirv.bat");
		system("cd data/shaders/colored/ && generate-spirv.bat");
		system("cd data/shaders/starsphere/ && generate-spirv.bat");
		//system("cls");
	}

	void VulkanApp::LoadModels()
	{
		// Create the camera
		mCamera = new Camera(glm::vec3(500, 1300, 500), 60.0f, (float)GetWindowWidth() / (float)GetWindowHeight(), 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 0, 0));

		// Load the starsphere
		Object* sphere = new Object(glm::vec3(0, 0, 0));
		sphere->SetModel(mModelLoader.LoadModel(this, "data/models/sphere.obj"));
		sphere->SetScale(glm::vec3(100));
		sphere->SetPipeline(mPipelines.starsphere);
		sphere->SetId(OBJECT_ID_SKY);
		mObjects.push_back(sphere);

		// Load a random testing texture
		mTextureLoader->loadTexture("data/textures/crate_bc3.dds", VK_FORMAT_BC3_UNORM_BLOCK, &mTestTexture);
		mTextureLoader->loadTexture("data/textures/bricks.dds", VK_FORMAT_BC3_UNORM_BLOCK, &mTerrainTexture);

		Object* terrain = new Object(glm::vec3(-1000, 0, -1000));
		terrain->SetModel(mModelLoader.GenerateTerrain(this, "data/textures/fft-terrain.tga"));
		terrain->SetPipeline(mPipelines.colored);
		terrain->SetScale(glm::vec3(10, 10, 10));
		terrain->SetColor(glm::vec3(0.0, 0.9, 0.0));
		terrain->SetId(OBJECT_ID_TERRAIN);
		mObjects.push_back(terrain);

		// Generate some positions
		int size = 6;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				for (int z = 0; z < size; z++)
				{
					Object* object = new Object(glm::vec3(x * 150, -100 - y * 150, z * 150));
					object->SetScale(glm::vec3((rand() % 20) / 10.0f));
					object->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
					object->SetId(OBJECT_ID_PROP);

					if (rand() % 2 == 0) {
						object->SetModel(mModelLoader.LoadModel(this, "data/models/teapot.3ds"));
						object->SetRotation(glm::vec3(180, 0, 0));
						object->SetPipeline(mPipelines.colored);
					}
					else {
						object->SetModel(mModelLoader.LoadModel(this, "data/models/box.obj"));
						object->SetPipeline(mPipelines.textured);
						object->SetScale(glm::vec3(4.0f));
					}

					mObjects.push_back(object);
				}
			}
		}

		// TODO: Needs setup the binding descriptions
	}

	void VulkanApp::PrepareUniformBuffers()
	{
		// Create the uniform buffer
		// It's the same process as creating any buffer, except that the VkBufferCreateInfo.usage bit is different (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		VkBufferCreateInfo createInfo = {};
		VkMemoryAllocateInfo allocInfo = {};
		VkMemoryRequirements memoryRequirments = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = 0;								// Gets assigned with vkGetBufferMemoryRequirements
		allocInfo.memoryTypeIndex = 0;

		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(mUniformData);						// 3x glm::mat4 NOTE: Not any more!!
		createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VulkanDebug::ErrorCheck(vkCreateBuffer(mDevice, &createInfo, nullptr, &mUniformBuffer.buffer));
		vkGetBufferMemoryRequirements(mDevice, mUniformBuffer.buffer, &memoryRequirments);
		allocInfo.allocationSize = memoryRequirments.size;
		GetMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);
		VulkanDebug::ErrorCheck(vkAllocateMemory(mDevice, &allocInfo, nullptr, &mUniformBuffer.memory));
		VulkanDebug::ErrorCheck(vkBindBufferMemory(mDevice, mUniformBuffer.buffer, mUniformBuffer.memory, 0));

		// uniformBuffer.buffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
		// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
		mUniformBuffer.descriptor.buffer = mUniformBuffer.buffer;
		mUniformBuffer.descriptor.offset = 0;
		mUniformBuffer.descriptor.range = sizeof(mUniformData);		// 3x glm::mat4

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
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = layoutBinding.size();
		createInfo.pBindings = layoutBinding.data();

		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(mDevice, &createInfo, nullptr, &mDescriptorSetLayout));

		// Create the pipeline layout that will use the descriptor set layout
		// The pipeline layout is used later when creating the pipeline
		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
		pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pPipelineLayoutCreateInfo.pNext = NULL;
		pPipelineLayoutCreateInfo.setLayoutCount = 1;
		pPipelineLayoutCreateInfo.pSetLayouts = &mDescriptorSetLayout;

		// Add push constants for the MVP matrix
		VkPushConstantRange pushConstantRanges = {};
		pushConstantRanges.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		pushConstantRanges.offset = 0;
		pushConstantRanges.size = sizeof(PushConstantBlock);

		pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRanges;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
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

		VulkanDebug::ErrorCheck(vkCreateDescriptorPool(mDevice, &createInfo, nullptr, &mDescriptorPool));
	}

	void VulkanApp::SetupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;					// 2 - 1 for the terrain texture
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet));

		VkDescriptorImageInfo texDescriptor = {};
		texDescriptor.sampler = mTestTexture.sampler;				// NOTE: TODO: This feels really bad, not scalable with more objects at all, fix!!! LoadModel() must run before this!!
		texDescriptor.imageView = mTestTexture.view;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		// Binding 0 : Uniform buffer
		std::vector<VkWriteDescriptorSet> writeDescriptorSet(2);
		writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[0].dstSet = mDescriptorSet;
		writeDescriptorSet[0].descriptorCount = 1;
		writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet[0].pBufferInfo = &mUniformBuffer.descriptor;
		writeDescriptorSet[0].dstBinding = 0;				// Binds this uniform buffer to binding point 0

		//  Binding 1: Image sampler
		writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[1].dstSet = mDescriptorSet;
		writeDescriptorSet[1].descriptorCount = 1;
		writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet[1].pImageInfo = &texDescriptor;
		writeDescriptorSet[1].dstBinding = 1;				// Binds the image sampler to binding point 1

		vkUpdateDescriptorSets(mDevice, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, NULL);

		//
		// Terrain descriptor set
		// 
		/*texDescriptor.sampler = terrainTexture.sampler;				// NOTE: TODO: This feels really bad, not scalable with more objects at all, fix!!! LoadModel() must run before this!!
		texDescriptor.imageView = terrainTexture.view;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		writeDescriptorSet[0].dstSet = descriptorSet[1];
		writeDescriptorSet[1].dstSet = descriptorSet[1];

		vkUpdateDescriptorSets(device, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, NULL);*/
	}

	void VulkanApp::SetupTerrainDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice, &allocInfo, &mTerrainDescriptorSet));

		VkDescriptorImageInfo texDescriptor = {};
		texDescriptor.sampler = mTerrainTexture.sampler;				// NOTE: TODO: This feels really bad, not scalable with more objects at all, fix!!! LoadModel() must run before this!!
		texDescriptor.imageView = mTerrainTexture.view;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		// Binding 0 : Uniform buffer
		std::vector<VkWriteDescriptorSet> writeDescriptorSet(2);
		writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[0].dstSet = mTerrainDescriptorSet;
		writeDescriptorSet[0].descriptorCount = 1;
		writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet[0].pBufferInfo = &mUniformBuffer.descriptor;
		writeDescriptorSet[0].dstBinding = 0;				// Binds this uniform buffer to binding point 0

		//  Binding 1: Image sampler
		writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[1].dstSet = mTerrainDescriptorSet;
		writeDescriptorSet[1].descriptorCount = 1;
		writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet[1].pImageInfo = &texDescriptor;
		writeDescriptorSet[1].dstBinding = 1;				// Binds the image sampler to binding point 1

		vkUpdateDescriptorSets(mDevice, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, NULL);
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
		shaderStages[0] = LoadShader("data/shaders/textured/textured.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("data/shaders/textured/textured.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		// Assign all the states to the pipeline
		// The states will be static and can't be changed after the pipeline is created
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = mPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass;
		pipelineCreateInfo.pVertexInputState = &mVertexDescriptions.inputState;		// From base - &vertices.inputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();

		// Create the solid pipeline
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipelines.textured));

		// Create the wireframe pipeline
		shaderStages[1] = LoadShader("data/shaders/colored/colored.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipelines.colored));

		// Create the starsphere pipeline
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		depthStencilState.depthWriteEnable = VK_FALSE;
		shaderStages[0] = LoadShader("data/shaders/starsphere/starsphere.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("data/shaders/starsphere/starsphere.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		vkTools::checkResult(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipelines.starsphere));
	}

	// Call this every time any uniform buffer should be updated (view changes etc.)
	void VulkanApp::UpdateUniformBuffers()
	{
		mUniformData.projectionMatrix = mCamera->GetProjection(); // glm::perspective(glm::radians(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 256.0f);

		float zoom = -8;
		glm::mat4 viewMatrix = mCamera->GetView(); //camera->GetViewMatrix();// glm::mat4();

		mUniformData.viewMatrix = mCamera->GetView();
		mUniformData.projectionMatrix = mCamera->GetProjection();
		mUniformData.eyePos = mCamera->GetPosition();

		/*uniformData.modelMatrix = glm::mat4();
		uniformData.modelMatrix = viewMatrix * glm::translate(uniformData.modelMatrix, modelPos);
		uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		uniformData.modelMatrix = glm::rotate(uniformData.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));*/


		// Map uniform buffer and update it
		uint8_t *data;
		VulkanDebug::ErrorCheck(vkMapMemory(mDevice, mUniformBuffer.memory, 0, sizeof(mUniformData), 0, (void **)&data));
		memcpy(data, &mUniformData, sizeof(mUniformData));
		vkUnmapMemory(mDevice, mUniformBuffer.memory);
	}

	void VulkanApp::SetupVertexDescriptions()
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescriptions.bindingDescriptions.resize(1);
		mVertexDescriptions.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;				// Bind to ID 0, this information will be used by the shader
		mVertexDescriptions.bindingDescriptions[0].stride = sizeof(Vertex);						// Size of each vertex
		mVertexDescriptions.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescriptions.attributeDescriptions.resize(5);

		// Location 0 : Position
		mVertexDescriptions.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
		mVertexDescriptions.attributeDescriptions[0].location = 0;								// Location 0 (will be used in the shader)
		mVertexDescriptions.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		mVertexDescriptions.attributeDescriptions[0].offset = 0;									// First attribute can start at offset 0
		mVertexDescriptions.attributeDescriptions[0].binding = 0;

		// Location 1 : Color
		mVertexDescriptions.attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
		mVertexDescriptions.attributeDescriptions[1].location = 1;								// Location 1 (will be used in the shader)
		mVertexDescriptions.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		mVertexDescriptions.attributeDescriptions[1].offset = sizeof(float) * 3;					// Second attribute needs to start with offset = sizeof(attribute 1)
		mVertexDescriptions.attributeDescriptions[1].binding = 0;

		// Location 2 : Normal
		mVertexDescriptions.attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
		mVertexDescriptions.attributeDescriptions[2].location = 2;
		mVertexDescriptions.attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		mVertexDescriptions.attributeDescriptions[2].offset = sizeof(float) * 6;
		mVertexDescriptions.attributeDescriptions[2].binding = 0;

		// Location 3 : Texture
		mVertexDescriptions.attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
		mVertexDescriptions.attributeDescriptions[3].location = 3;
		mVertexDescriptions.attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		mVertexDescriptions.attributeDescriptions[3].offset = sizeof(float) * 9;
		mVertexDescriptions.attributeDescriptions[3].binding = 0;

		// Location 4 : Tangent
		mVertexDescriptions.attributeDescriptions[4].binding = VERTEX_BUFFER_BIND_ID;
		mVertexDescriptions.attributeDescriptions[4].location = 4;
		mVertexDescriptions.attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		mVertexDescriptions.attributeDescriptions[4].offset = sizeof(float) * 11;
		mVertexDescriptions.attributeDescriptions[4].binding = 0;

		// Neither the bindingDescriptions or the attributeDescriptions is used directly
		// When creating a graphics pipeline a VkPipelineVertexInputStateCreateInfo structure is sent as an argument and this structure
		// contains the VkVertexInputBindingDescription and VkVertexInputAttributeDescription
		// The last thing to do is to assign the binding and attribute descriptions
		mVertexDescriptions.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mVertexDescriptions.inputState.pNext = NULL;
		mVertexDescriptions.inputState.vertexBindingDescriptionCount = mVertexDescriptions.bindingDescriptions.size();
		mVertexDescriptions.inputState.pVertexBindingDescriptions = mVertexDescriptions.bindingDescriptions.data();
		mVertexDescriptions.inputState.vertexAttributeDescriptionCount = mVertexDescriptions.attributeDescriptions.size();
		mVertexDescriptions.inputState.pVertexAttributeDescriptions = mVertexDescriptions.attributeDescriptions.data();
	}

	void VulkanApp::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clearValues[2];
		clearValues[0].color = { 1.0f, 0.8f, 0.4f, 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass;
		renderPassBeginInfo.renderArea.extent.width = GetWindowWidth();
		renderPassBeginInfo.renderArea.extent.height = GetWindowHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer;

		// Begin command buffer recording & the render pass
		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPrimaryCommandBuffer, &beginInfo));
		vkCmdBeginRenderPass(mPrimaryCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//
		// Secondary command buffer
		//
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = mRenderPass;
		inheritanceInfo.framebuffer = frameBuffer;

		// Secondary command buffer for the sky sphere
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mSecondaryCommandBuffer, &commandBufferBeginInfo));

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = (float)GetWindowWidth();
		viewport.height = (float)GetWindowHeight();
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(mSecondaryCommandBuffer, 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = GetWindowWidth();
		scissor.extent.height = GetWindowHeight();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(mSecondaryCommandBuffer, 0, 1, &scissor);

		//
		// Testing push constant rendering with different matrices
		//
		for (auto& object : mObjects)
		{

			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->GetPipeline());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, NULL);

			// Push the world matrix constant
			mPushConstants.world = object->GetWorldMatrix(); // camera->GetProjection() * camera->GetView() * 
			mPushConstants.color = object->GetColor();
			vkCmdPushConstants(mSecondaryCommandBuffer, mPipelineLayout, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, sizeof(PushConstantBlock), &mPushConstants);

			
			// Bind triangle vertices
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(mSecondaryCommandBuffer, VERTEX_BUFFER_BIND_ID, 1, &object->GetModel()->vertices.buffer, offsets);		// [TODO] The renderer should group the same object models together
			vkCmdBindIndexBuffer(mSecondaryCommandBuffer, object->GetModel()->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw indexed triangle	
			vkCmdSetLineWidth(mSecondaryCommandBuffer, 1.0f);
			vkCmdDrawIndexed(mSecondaryCommandBuffer, object->GetModel()->GetNumIndices(), 1, 0, 0, 0);
		}

		// End secondary command buffer
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mSecondaryCommandBuffer));

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(mPrimaryCommandBuffer, 1, &mSecondaryCommandBuffer);

		// End command buffer recording & the render pass
		vkCmdEndRenderPass(mPrimaryCommandBuffer);
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPrimaryCommandBuffer));

	}

	void VulkanApp::Draw()
	{
		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 

		// Acquire the next image in the swap chain
		VulkanDebug::ErrorCheck(mSwapChain.acquireNextImage(mPresentComplete, &mCurrentBuffer));

		//
		// Transition image format to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//
		SubmitPrePresentMemoryBarrier(mSwapChain.buffers[mCurrentBuffer].image);

		// NOTE: Testing
		RecordRenderingCommandBuffer(mFrameBuffers[mCurrentBuffer]);

		//
		// Do rendering
		//

		// Submit the recorded draw command buffer to the queue
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mPrimaryCommandBuffer;		// Draw commands for the current command buffer
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mPresentComplete;							// Waits for swapChain.acquireNextImage to complete
		submitInfo.pSignalSemaphores = &mRenderComplete;							// swapChain.queuePresent will wait for this submit to complete
		VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submitInfo.pWaitDstStageMask = &stageFlags;

		VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE));

		//
		// Transition image format to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		//

		SubmitPostPresentMemoryBarrier(mSwapChain.buffers[mCurrentBuffer].image);

		// Present the image
		VulkanDebug::ErrorCheck(mSwapChain.queuePresent(mQueue, mCurrentBuffer, mRenderComplete));
	}

	void VulkanApp::Render()
	{
		//if (!prepared)
		//	return;

		vkDeviceWaitIdle(mDevice);		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!

		mCamera->Update();

		// NOTE: TODO: TESTING
		if (mPrepared)
			UpdateUniformBuffers();

		Draw();

		vkDeviceWaitIdle(mDevice);		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!
	}

	void VulkanApp::Update()
	{
		// Rotate the objects
		for (auto& object : mObjects)
		{
			// [NOTE] Just for testing
			float speed = 50.0f;
			if(object->GetId() == OBJECT_ID_PROP)
				object->AddRotation(glm::radians(speed), glm::radians(speed), glm::radians(speed));
		}
	}

	void VulkanApp::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Default message handling
		VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);

		// Let the camera handle user input
		mCamera->HandleMessages(hwnd, msg, wParam, lParam);
	}
}	// VulkanLib namespace