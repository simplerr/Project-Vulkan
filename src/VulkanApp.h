#pragma once

#include "VulkanBase.h"
#include "ModelLoader.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class StaticModel;
class Camera;
class Object;
class TextureData;

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

	void SetupVertexDescriptions();

	void RecordRenderingCommandBuffer();

	virtual void Render();
	void Draw();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 
	//	High level code
	//
	void LoadModels();
	void SetupTerrainDescriptorSet();

	// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
	// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
	// inputState is the pVertexInputState when creating the graphics pipeline
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertexDescriptions;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	} uniformBuffer;

	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::vec4 lightDir = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
		glm::vec3 eyePos;
	} uniformData;		// Stored in uniformBuffer.memory in device memory

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;

	struct {
		VkPipeline textured;
		VkPipeline colored;
		VkPipeline starsphere;
	} pipelines;

	// 
	//	High level code
	//

	VkDescriptorSet terrainDescriptorSet;

	ModelLoader modelLoader;
	vkTools::VulkanTexture testTexture;		// NOTE: just for testing
	vkTools::VulkanTexture terrainTexture;	// Testing for the terrain
	Camera* camera;

	bool prepared = false;

	std::vector<Object*>  mObjects;
};
