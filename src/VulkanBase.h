#pragma once

#pragma comment(linker, "/subsystem:windows")

//#include "../external\vulkan\vulkan.h"
#include "../base/vulkantools.h"	
#include "../base/vulkanswapchain.hpp"

#include <vulkan\vulkan.h>


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
	HWND CreateWin32Window(HINSTANCE hInstance, WNDPROC wndProc);

	void InitSwapchain();
	void SetupSwapchain();

	void RenderLoop();
	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void exitOnError(const char* msg);
private:
	VkInstance			instance;
	VkPhysicalDevice	physicalDevice;
	VkDevice			device;

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

	//VkDebugReportCallbackEXT msgCallback = nullptr;
	//VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
};

