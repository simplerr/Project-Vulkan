#if defined(_WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <array>
#include <cassert>
#include <sstream>

#include "VulkanBase.h"

VulkanBase::VulkanBase()
{
	setupConsole("Debug console");
	setupDebugLayers();

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

	initDebug();

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
	vkDestroyInstance(instance, nullptr);
	cleanupDebug();
}

// Win32 : Sets up a console window and redirects standard output to it
void VulkanBase::setupConsole(std::string title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle(TEXT(title.c_str()));

	std::cout << "Debug console:\n";
}

void VulkanBase::exitOnError(const char* msg) {
	fputs(msg, stderr);
	exit(EXIT_FAILURE);
}

VKAPI_ATTR VkBool32 VKAPI_CALL 
VulkanDebugCallback(
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

// Debug extension callbacks
PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = nullptr;
PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = nullptr;
PFN_vkDebugReportMessageEXT dbgBreakCallback = nullptr;

void VulkanBase::initDebug()
{
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");

	if (CreateDebugReportCallback == nullptr || DestroyDebugReportCallback == nullptr || dbgBreakCallback == nullptr) {
		exitOnError("Error fetching debug function pointers");
	}

	CreateDebugReportCallback(instance, &debugCallbackCreateInfo, nullptr, &msgCallback);
}

void VulkanBase::setupDebugLayers()
{
	// TODO: Add #ifdef _DEBUG
	/* Setup callback creation information */
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

	validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
}

void VulkanBase::cleanupDebug()
{
	DestroyDebugReportCallback(instance, msgCallback, nullptr);
	msgCallback = nullptr;
}

VkResult VulkanBase::createInstance(const char* appName)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;	// must be VK_STRUCTURE_TYPE_APPLICATION_INFO
	appInfo.pNext = nullptr;	// must be NULL
	appInfo.pApplicationName = appName;
	appInfo.pEngineName = appName;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);	// all drivers support this, but use VK_API_VERSION in the future

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
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // must be VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
	createInfo.pNext = &debugCallbackCreateInfo; // enables debugging when creating the instance
	createInfo.flags = 0; // must be 0
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	createInfo.enabledLayerCount = validation_layers.size();	// debug layers
	createInfo.ppEnabledLayerNames = validation_layers.data();

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
	deviceInfo.enabledExtensionCount = enabledExtensions.size();	// extensions
	deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	deviceInfo.enabledLayerCount = validation_layers.size();	// debug layers
	deviceInfo.ppEnabledLayerNames = validation_layers.data();
	


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