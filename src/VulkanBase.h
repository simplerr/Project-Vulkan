#pragma once
#pragma comment(linker, "/subsystem:windows")

//#include "../external\vulkan\vulkan.h"
#include "../base/vulkantools.h"	
#include "../base/vulkanswapchain.hpp"
#include "../base/vulkanTextureLoader.hpp"

#include <vulkan\vulkan.h>

/*
 Resources

	Validation layer guide: http://gpuopen.com/using-the-vulkan-validation-layers/
	Vulkan specification: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html
	Vulkan spec + WSI: https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html
	Vulkan in 30 minutes: https://renderdoc.org/vulkan-in-30-minutes.html
	Memory management: https://developer.nvidia.com/vulkan-memory-management
	Niko Kauppi videoes: https://www.youtube.com/watch?v=Bu581jeyTL0
	Pipeleline barriers: https://github.com/philiptaylor/vulkan-sxs/tree/master/04-clear
	Intel Vulkan tutorial: https://software.intel.com/en-us/api-without-secrets-introduction-to-vulkan-part-2
*/


// This is the base class that contains common code for creating a Vulkan application
class VulkanBase
{
public:
	VulkanBase();
	~VulkanBase();

	VkResult CreateInstance(const char* appName);
	VkResult CreateDevice();

	virtual void Prepare();

	void CreateCommandPool();
	void CreateSetupCommandBuffer();
	void CreateCommandBuffers();
	void CreateSemaphores();

	void SetupDepthStencil();
	void SetupRenderPass();
	void SetupFrameBuffer();
	// Don't need pipeline cache

	void InitSwapchain();
	void SetupSwapchain();

	void ExecuteSetupCommandBuffer();

	VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);

	virtual void Render() = 0;

	// To transition the swap chain image layout
	void SubmitPrePresentMemoryBarrier(VkImage image);
	void SubmitPostPresentMemoryBarrier(VkImage image);

	VkBool32 GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex);

	void RenderLoop();
	virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND CreateWin32Window(HINSTANCE hInstance, WNDPROC wndProc);

	VkDevice GetDevice();

protected:
	VkInstance			instance				= VK_NULL_HANDLE;
	VkPhysicalDevice	physicalDevice			= VK_NULL_HANDLE;
	VkDevice			device					= VK_NULL_HANDLE;
	VkQueue				queue					= VK_NULL_HANDLE;

	// Command buffer
	VkCommandPool		commandPool;
	std::vector<VkCommandBuffer> renderingCommandBuffers;

	// Command buffer used for setup
	VkCommandBuffer		setupCmdBuffer			= VK_NULL_HANDLE;

	// Command buffers used to change the swapchains image format
	VkCommandBuffer		postPresentCmdBuffer	= VK_NULL_HANDLE;
	VkCommandBuffer		prePresentCmdBuffer		= VK_NULL_HANDLE;

	// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
	VulkanSwapChain		swapChain;

	// Global render pass for frame buffer writes
	VkRenderPass		renderPass;

	VkSemaphore			presentComplete;
	VkSemaphore			renderComplete;

	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer>	frameBuffers;

	// Active frame buffer index
	uint32_t			currentBuffer			= 0;

	// Hardcoded for now, should be selected during init with proper tests
	VkFormat			depthFormat				= VK_FORMAT_D32_SFLOAT_S8_UINT;

	// Color buffer format
	VkFormat			colorformat				= VK_FORMAT_B8G8R8A8_UNORM;

	// Descriptor set pool
	VkDescriptorPool	descriptorPool			= VK_NULL_HANDLE;

	// List of shader modules created and that needs cleanup
	std::vector<VkShaderModule> shaderModules;

	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

	// Group everything with the depth stencil together in a struct (as in Vulkan samples)
	struct {		
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	} depthStencil;

	// Simple texture loader
	vkTools::VulkanTextureLoader *textureLoader	= nullptr;

	// Last frame time, measured using a high performance timer (if available)
	float frameTimer = 1.0f;

	// Frame counter to display fps
	uint32_t frameCounter = 0;

	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;

	// fps timer (one second interval)
	float fpsTimer = 0.0f;
	
#if defined(_WIN32)
	HWND				window;
	HINSTANCE			windowInstance;
#endif

	uint32_t			windowWidth = 1024;
	uint32_t			windowHeight = 768;
};

