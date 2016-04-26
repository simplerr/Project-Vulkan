#pragma once
#include "VulkanBase.h"
#include "ModelLoader.h"
#include "ThreadPool.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

using namespace glm;

namespace VulkanLib
{
	class StaticModel;
	class Camera;
	class Object;
	class TextureData;
	class Light;

	struct VertexDescriptions {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	struct UniformBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	};

	struct UniformData {
		struct {
			mat4 projectionMatrix;
			mat4 viewMatrix;
			vec4 lightDir = vec4(1.0f, -1.0f, 1.0f, 1.0f);
			vec3 eyePos;	
			float t;

			
		} camera;

		
		//vec4* test;
		
		std::vector<Light> lights;
		

		//std::vector<Light*> lights;

	

		// Array of world matrixes for the instances
		//mat4* instanceWorld;
	};	// Stored in uniformBuffer.memory in device memory

	struct Pipelines {
		VkPipeline textured;
		VkPipeline colored;
		VkPipeline starsphere;
	};

	struct PushConstantBlock {
		mat4 world;
		vec3 color;
	};

	struct ThreadData {
		PushConstantBlock pushConstants;
		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;
		VkDescriptorSet descriptorSet;			// Testing
		VkDescriptorPool descriptorPool;
		std::vector<Object*> threadObjects;
	};

	class VulkanApp : public VulkanBase
	{
	public:
		VulkanApp();
		~VulkanApp();

		void Prepare();

		void PrepareUniformBuffers();
		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void SetupDescriptorSet();
		void PreparePipelines();
		void UpdateUniformBuffers();
		void PrepareCommandBuffers();		// Custom
		void SetupVertexDescriptions();

		void SetupMultithreading();			// Custom

		void BuildInstancingCommandBuffer(VkFramebuffer frameBuffer);
		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);
		void ThreadRecordCommandBuffer(int threadId, VkCommandBufferInheritanceInfo inheritanceInfo);

		virtual void Render();
		virtual void Update();
		void Draw();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 
		//	High level code
		//
		void LoadModels();
		void SetupTerrainDescriptorSet();
		void CompileShaders();


		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescriptions				mVertexDescriptions;
		UniformBuffer					mUniformBuffer;
		UniformData						mUniformData;		// Stored in mUniformBuffer.memory in device memory
		Pipelines						mPipelines;

		VkDescriptorSetLayout			mDescriptorSetLayout;
		VkDescriptorSet					mDescriptorSet;
		VkPipelineLayout				mPipelineLayout;

		// This gets regenerated each frame so there is no need for command buffer per frame buffer
		VkCommandBuffer					mPrimaryCommandBuffer;
		VkCommandBuffer					mSecondaryCommandBuffer;

		// 
		//	High level code
		//

		VkDescriptorSet					mTerrainDescriptorSet;
		PushConstantBlock				mPushConstants;		// Gets updated with new push constants for each object
	
		vkTools::VulkanTexture			mTestTexture;		// NOTE: just for testing
		vkTools::VulkanTexture			mTerrainTexture;	// Testing for the terrain
		
		bool							mPrepared = false;

		Camera*							mCamera;
		ModelLoader						mModelLoader;
		std::vector<Object*>			mObjects;

		std::vector<ThreadData>			mThreadData;
		int								mNumThreads;
		int								mNumObjects;
		ThreadPool						mThreadPool;
	};
}	// VulkanLib namespace
