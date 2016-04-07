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

	VkResult createInstance(const char* appName);
	VkResult createDevice();
	HWND createWindow(HINSTANCE hInstance, WNDPROC wndProc);
	void setupConsole(std::string title);

	void initSwapchain();
	void setupSwapchain();

	void setupDebugLayers();
	void initDebug();
	void cleanupDebug();

	void renderLoop();
	void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void exitOnError(const char* msg);
private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	// Swap chain magic by Sascha Willems (https://github.com/SaschaWillems/Vulkan)
	VulkanSwapChain swapChain;

	// Command buffer used for setup
	VkCommandBuffer setupCmdBuffer = VK_NULL_HANDLE;

#if defined(_WIN32)
	HWND window;
	HINSTANCE windowInstance;
#endif

	// Debugging layers
	std::vector<const char*> validation_layers;

	uint32_t windowWidth = 1024;
	uint32_t windowHeight = 768;

	VkDebugReportCallbackEXT msgCallback = nullptr;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
};

