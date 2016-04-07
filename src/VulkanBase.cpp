#if defined(_WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <array>
#include <cassert>
#include <sstream>

#include "VulkanBase.h"
#include "VulkanDebug.h"

VulkanBase::VulkanBase()
{
	VulkanDebug::SetupDebugLayers();

	VkResult res;

	res = createInstance("Vulkan App");

	// TODO: write own debug output functions
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
		exitOnError(
			"Cannot find a compatible Vulkan installable client "
			"driver (ICD). Please make sure your driver supports "
			"Vulkan before continuing. The call to vkCreateInstance failed.");
	}
	else if (res != VK_SUCCESS) {
		exitOnError(
			"The call to vkCreateInstance failed. Please make sure "
			"you have a Vulkan installable client driver (ICD) before "
			"continuing.");
	}

	VulkanDebug::InitDebug(instance);

	res = createDevice();

	if (res != VK_SUCCESS) {
		exitOnError("Error creating the device");
	}

	// Setup function pointers for the swapchain
	//swapChain.connect(instance, physicalDevice, device);

	// Init the surface
	//initSwapchain();
}

VulkanBase::~VulkanBase()
{
	//swapChain.cleanup();
	vkDestroyDevice(device, nullptr);

	VulkanDebug::CleanupDebugging(instance);

	vkDestroyInstance(instance, nullptr);
}

void VulkanBase::exitOnError(const char* msg) {
	fputs(msg, stderr);
	exit(EXIT_FAILURE);
}

VkResult VulkanBase::createInstance(const char* appName)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// Must be VK_STRUCTURE_TYPE_APPLICATION_INFO
	appInfo.pNext = nullptr;									// Must be NULL
	appInfo.pApplicationName = appName;
	appInfo.pEngineName = appName;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);				// All drivers support this, but use VK_API_VERSION in the future

	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Extension for the Win32 surface 
#if defined(_WIN32)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	// Add the debug extension
	enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// Must be VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
	createInfo.pNext = &VulkanDebug::debugCallbackCreateInfo;	// Enables debugging when creating the instance
	createInfo.flags = 0;										// Must be 0
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = enabledExtensions.size();			// Extensions
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.enabledLayerCount = VulkanDebug::validation_layers.size();	// Debug validation layers
	createInfo.ppEnabledLayerNames = VulkanDebug::validation_layers.data();

	VkResult res = vkCreateInstance(&createInfo, NULL, &instance);

	return res;
}

VkResult VulkanBase::createDevice()
{
	// Query for the number of GPUs
	uint32_t gpuCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);

	if (result != VK_SUCCESS)
		exitOnError("vkEnumeratePhysicalDevices failed");

	if (gpuCount < 1)
		exitOnError("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());

	// Assume that there only is 1 GPU
	physicalDevice = physicalDevices[0];

	// This is not used right now, but GPU vendor and model can be retrieved
	//VkPhysicalDeviceProperties physicalDeviceProperties;
	//vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	// Some implementations use vkGetPhysicalDeviceQueueFamilyProperties and uses the result to find out
	// the first queue that support graphic operations (queueFlags & VK_QUEUE_GRAPHICS_BIT)
	// Here I simply set queueInfo.queueFamilyIndex = 0 and (hope) it works

	std::array<float, 1> queuePriorities = { 1.0f };
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // must
	queueInfo.pNext = nullptr;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = 0; // 0 seems to allways be the first valid queue (see above)
	queueInfo.pQueuePriorities = queuePriorities.data();
	queueInfo.queueCount = 1;

	std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.pEnabledFeatures = nullptr;
	deviceInfo.enabledExtensionCount = enabledExtensions.size();			// Extensions
	deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	deviceInfo.enabledLayerCount = VulkanDebug::validation_layers.size();	// Debug validation layers
	deviceInfo.ppEnabledLayerNames = VulkanDebug::validation_layers.data();
	
	result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);

	return result;
}

// Creates a window that Vulkan can use for rendering
HWND VulkanBase::createWindow(HINSTANCE hInstance, WNDPROC WndProc)
{
	windowInstance = hInstance;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = windowInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "VulkanWndClassName";

	if (!RegisterClass(&wc)) {
		MessageBox(0, "RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	RECT clientRect;
	clientRect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2.0f;
	clientRect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + windowWidth / 2.0f;
	clientRect.top = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2.0f;
	clientRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2 + windowHeight / 2.0f;

	DWORD style = true ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN : WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN;

	AdjustWindowRect(&clientRect, style, false);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Create the window with a custom size and make it centered
	// NOTE: WS_CLIPCHILDREN Makes the area under child windows not be displayed. (Useful when rendering DirectX and using windows controls).
	window = CreateWindow("VulkanWndClassName", "Vulkan App",
		style, GetSystemMetrics(SM_CXSCREEN) / 2 - (windowWidth / 2),
		GetSystemMetrics(SM_CYSCREEN) / 2 - (windowHeight / 2), width, height,
		0, 0, windowInstance, 0);

	if (!window) {
		auto error = GetLastError();
		MessageBox(0, "CreateWindow() failed.", 0, 0);
		PostQuitMessage(0);
	}

	// Show the newly created window.
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	return window;
}

void VulkanBase::initSwapchain()
{
	// Platform dependent code to initialize the window surface
#if defined(_WIN32)
	//swapChain.initSurface(windowInstance, window);
#endif
}

void VulkanBase::setupSwapchain()
{
	//swapChain.create(setupCmdBuffer, &windowWidth, &windowHeight);
}

void VulkanBase::renderLoop()
{
	MSG message;

	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void VulkanBase::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		PostQuitMessage(0);
		break;
	}
}