#pragma once
#include "VulkanBase.h"
#include "ModelLoader.h"
#include "ThreadPool.h"
#include "StaticModel.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VertexDescription.h"
#include "UniformBuffer.h"
#include "BigUniformBuffer.h"
#include "DescriptorSet.h"

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

	struct Buffer {
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

		std::vector<Light> lights;

		struct {
			float numLights;
			bool useInstancing;
			vec2 garbage;
		} constants;
		

		//std::vector<Light*> lights;

		// Array of world matrixes for the instances
		//mat4* instanceWorld;
	};	// Stored in uniformBuffer.memory in device memory

	struct InstanceData {
		vec3 position;
		vec3 scale;
		vec3 color;
	};

	struct Pipelines {
		VkPipeline textured;
		VkPipeline colored;
		VkPipeline starsphere;
		VkPipeline instanced;
	};

	struct PushConstantBlock {
		mat4 world;
		vec3 color;
	};

	struct VulkanModel
	{
		Object* object;
		StaticModel* mesh;
		VkPipeline pipeline;

	};

	struct ThreadData {
		PushConstantBlock pushConstants;
		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;
		VkDescriptorSet descriptorSet;			// Testing
		VkDescriptorPool descriptorPool;
		std::vector<VulkanModel> threadObjects;

		StaticModel model;
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

		void SetupMultithreading(int numThreads);			// Custom
		void EnableInstancing(bool useInstancing);
		void PrepareInstancing();

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
		void SetCamera(Camera* camera);

		void AddModel(VulkanModel model);


		//UniformBuffer					mUniformBuffer;
		//UniformData						mUniformData;		// Stored in mUniformBuffer.memory in device memory
		Pipelines						mPipelines;

		//VkDescriptorSetLayout			mDescriptorSetLayout;
		VkDescriptorSet					mDescriptorSet;
		VkPipelineLayout				mPipelineLayout;

		// This gets regenerated each frame so there is no need for command buffer per frame buffer
		VkCommandBuffer					mPrimaryCommandBuffer;
		VkCommandBuffer					mSecondaryCommandBuffer;

		VkFence							mRenderFence = {};

		// 
		//	High level code
		//

		VkDescriptorSet					mTerrainDescriptorSet;
		PushConstantBlock				mPushConstants;		// Gets updated with new push constants for each object
	
		vkTools::VulkanTexture			mTestTexture;		// NOTE: just for testing
		vkTools::VulkanTexture			mTerrainTexture;	// Testing for the terrain
		
		bool							mPrepared = false;

		Buffer							mInstanceBuffer;
		bool							mUseInstancing;

		Camera*							mCamera;
		//ModelLoader					mModelLoader;
	//	std::vector<Object*>			mObjects;

		// Threads
		std::vector<ThreadData>			mThreadData;
		int								mNumThreads;
		int								mNumObjects;
		ThreadPool						mThreadPool;

		std::vector<VulkanModel>		mModels;

		int								mNextThreadId = 0;	// The thread to add new objects to

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescription				mVertexDescription;

		BigUniformBuffer				mUniformBuffer;

		DescriptorSet					mDescriptorSet1;

	public:
		StaticModel*					mTestModel;
	};
}	// VulkanLib namespace
