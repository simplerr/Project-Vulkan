#pragma once

#include "VulkanBase.h"
#include "ModelLoader.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class StaticModel;
class Camera;

class VulkanApp : public VulkanBase
{
public:
	VulkanApp();
	~VulkanApp();

	void Prepare();

	void PrepareVertices();
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

	// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertexDescriptions;


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

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	} uniformBuffer;

	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
	} uniformData;		// Stored in uniformBuffer.memory in device memory

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;

	VkPipeline pipeline;

	// 
	//	High level code
	//

	ModelLoader modelLoader;
	StaticModel* testModel;
	Camera* camera;

	bool prepared = false;
};
