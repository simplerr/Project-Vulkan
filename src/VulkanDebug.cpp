#include "VulkanDebug.h"
#include <sstream>
#include <iostream>

namespace VulkanDebug
{
	// Debug extension callbacks
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = nullptr;
	PFN_vkDebugReportMessageEXT dbgBreakCallback = nullptr;

	std::vector<const char*> validation_layers;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
	VkDebugReportCallbackEXT msgCallback = nullptr;

	void ErrorCheck(VkResult result)
	{

	}
	void SetupDebugLayers()
	{
		// Create a console to forward standard output to
		SetupConsole("Vulkan Debug Console");

		// TODO: Add #ifdef _DEBUG
		// Configure so that VulkanDebug::VulkanDebugCallback() gets all debug messages
		debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugCallbackCreateInfo.pNext = nullptr;
		debugCallbackCreateInfo.flags =
			//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

		debugCallbackCreateInfo.pfnCallback = &VulkanDebugCallback;
		debugCallbackCreateInfo.pUserData = nullptr;

		// Add all standard validation layers http://gpuopen.com/using-the-vulkan-validation-layers/
		validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	}
	void InitDebug(VkInstance instance)
	{
		CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");

		if (CreateDebugReportCallback == nullptr || DestroyDebugReportCallback == nullptr || dbgBreakCallback == nullptr) {
			//exitOnError("Error fetching debug function pointers");
		}

		ErrorCheck( CreateDebugReportCallback(instance, &debugCallbackCreateInfo, nullptr, &msgCallback) );
	}

	void CleanupDebugging(VkInstance instance)
	{
		DestroyDebugReportCallback(instance, msgCallback, nullptr);
		msgCallback = nullptr;
	}

	// Sets up a console window (Win32 only)
	void SetupConsole(std::string title)
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CON", "w", stdout);
		SetConsoleTitle(TEXT(title.c_str()));

		std::cout << "Debug console:\n";
	}

	// This is the callback that receives all debug messages from the different validation layers 
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugReportFlagsEXT       flags,
		VkDebugReportObjectTypeEXT  objectType,
		uint64_t                    object,
		size_t                      location,
		int32_t                     messageCode,
		const char*                 pLayerPrefix,
		const char*                 pMessage,
		void*                       pUserData)
	{
		std::ostringstream stream;

		stream << "VKDEBUG: ";
		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
			stream << "INFO: ";
		}
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			stream << "WARNING: ";
		}
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
			stream << "PERFORMANCE: ";
		}
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			stream << "ERROR: ";
		}
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
			stream << "DEBUG: ";
		}

		stream << "@[" << pLayerPrefix << "]: ";
		stream << pMessage << std::endl;

		std::cout << stream.str();

		#ifdef _WIN32
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			MessageBox(nullptr, stream.str().c_str(), "Vulkan Error!", 0);
		}
		#endif

		return VK_FALSE;
	}
}