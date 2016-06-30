#pragma once
#if defined(_WIN32)
#pragma comment(linker, "/subsystem:windows")
#endif

//#include "../external\vulkan\vulkan.h"
#include "base/vulkantools.h"	
#include "base/vulkanswapchain.hpp"
#include "base/vulkanTextureLoader.hpp"
#include "Window.h"
#include "Timer.h"

#include <vulkan/vulkan.h>

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
	Shader resource bindings: https://developer.nvidia.com/vulkan-shader-resource-binding
	Installation on Ubuntu: http://www.trentreed.net/blog/installing-nvidia-vulkan-driver-and-lunarg-sdk-on-ubuntu/

	https://github.com/jcouv/dotfiles/blob/master/vsvimrc
*/


namespace VulkanLib
{
	struct DepthStencil{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	// This is the base class that contains common code for creating a Vulkan application
	class VulkanBase
	{
	public:
		VulkanBase(bool enableValidation);
		~VulkanBase();

		VkResult CreateInstance(const char* appName, bool enableValidation);
		VkResult CreateDevice(bool enableValidation);

		virtual void Prepare();

		virtual void Update() = 0;
		virtual void Render() = 0;

		void CreateCommandPool();
		void CreateSetupCommandBuffer();
		void CreateCommandBuffers();
		void CreateSemaphores();

		void SetupDepthStencil();
		void SetupRenderPass();
		void SetupFrameBuffer();
		void BuildPresentCommandBuffers();
		// Don't need pipeline cache

		void InitSwapchain(Window* window);
		void SetupSwapchain();

		void ExecuteSetupCommandBuffer();

		VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);

		// To transition the swap chain image layout
		void SubmitPrePresentMemoryBarrier(VkImage image);
		void SubmitPostPresentMemoryBarrier(VkImage image);

		VkBool32 CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);

		void PrepareFrame();
		void SubmitFrame();

		VkBool32 GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex);

		virtual void CompileShaders() = 0;

		void RenderLoop();

		VkDevice GetDevice();
		int GetWindowWidth();
		int GetWindowHeight();

		// Platform specific
#if defined(_WIN32)
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	protected:
		VkInstance						mInstance					= VK_NULL_HANDLE;
		VkPhysicalDevice				mPhysicalDevice				= VK_NULL_HANDLE;
		VkDevice						mDevice						= VK_NULL_HANDLE;
		VkQueue							mQueue						= VK_NULL_HANDLE;

		// Command buffer
		VkCommandPool					mCommandPool;
		std::vector<VkCommandBuffer>	mRenderingCommandBuffers;

		// Command buffer used for setup
		VkCommandBuffer					mSetupCmdBuffer				= VK_NULL_HANDLE;

		// Command buffers used to change the swapchains image format
		//VkCommandBuffer					mPostPresentCmdBuffer		= VK_NULL_HANDLE;
		//VkCommandBuffer					mPrePresentCmdBuffer		= VK_NULL_HANDLE;

		// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
		VulkanSwapChain					mSwapChain;

		// Global render pass for frame buffer writes
		VkRenderPass					mRenderPass;

		VkSemaphore						mPresentComplete;
		VkSemaphore						mRenderComplete;

		// List of available frame buffers (same as number of swap chain images)
		std::vector<VkFramebuffer>		mFrameBuffers;

		// Active frame buffer index
		uint32_t						mCurrentBuffer				= 0;

		// Hardcoded for now, should be selected during init with proper tests
		VkFormat						mDepthFormat				= VK_FORMAT_D32_SFLOAT_S8_UINT;

		// Color buffer format
		VkFormat						mColorFormat				= VK_FORMAT_B8G8R8A8_UNORM;

		// Descriptor set pool
		VkDescriptorPool				mDescriptorPool				= VK_NULL_HANDLE;

		// List of shader modules created and that needs cleanup
		std::vector<VkShaderModule>		mShaderModules;

		// Stores all available memory (type) properties for the physical device
		VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;

		// Group everything with the depth stencil together in a struct (as in Vulkan samples)
		DepthStencil					mDepthStencil;

		// Simple texture loader
		vkTools::VulkanTextureLoader*	mTextureLoader = nullptr;

		// Wrapper class for the platform dependet window code
		Window*							mWindow;

		// Command buffer for submitting a post present image barrier
		std::vector<VkCommandBuffer>	mPostPresentCmdBuffers		= { VK_NULL_HANDLE };
		// Command buffers for submitting a pre present image barrier
		std::vector<VkCommandBuffer>	mPrePresentCmdBuffers		= { VK_NULL_HANDLE };
	};
}	// VulkanLib namespace
