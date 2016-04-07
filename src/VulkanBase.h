#pragma once

#pragma comment(linker, "/subsystem:windows")

//#include "../external\vulkan\vulkan.h"
#include "../base/vulkantools.h"	
#include "../base/vulkanswapchain.hpp"

#include <vulkan\vulkan.h>


// Validation layer guide: http://gpuopen.com/using-the-vulkan-validation-layers/

/*
	This is the base class that contains common code for creating a Vulkan application
*/
class VulkanBase
{
public:
	VulkanBase();
	~VulkanBase();

	VkResult CreateInstance(const char* appName);
	VkResult CreateDevice();

	void Prepare();

	void CreateCommandPool();
	void CreateSetupCommandBuffer();

	void InitSwapchain();
	void SetupSwapchain();

	void RenderLoop();
	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND CreateWin32Window(HINSTANCE hInstance, WNDPROC wndProc);

private:
	VkInstance			instance		= VK_NULL_HANDLE;
	VkPhysicalDevice	physicalDevice	= VK_NULL_HANDLE;
	VkDevice			device			= VK_NULL_HANDLE;

	// Command buffer
	VkCommandPool		commandPool;
	VkCommandBuffer		commandBuffer;

	// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
	VulkanSwapChain		swapChain;

	// Command buffer used for setup
	VkCommandBuffer		setupCmdBuffer = VK_NULL_HANDLE;

#if defined(_WIN32)
	HWND				window;
	HINSTANCE			windowInstance;
#endif

	uint32_t			windowWidth = 1024;
	uint32_t			windowHeight = 768;
};

