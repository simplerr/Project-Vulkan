#if defined(_WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <array>
#include <cassert>
#include <sstream>

#include "VulkanBase.h"
#include "VulkanDebug.h"

/*
	-	Right now this code assumes that queueFamilyIndex is = 0 in all places,
		no looping is done to find a queue that have the proper support
*/

VulkanBase::VulkanBase()
{
	VulkanDebug::SetupDebugLayers();

	// Create VkInstance
	VulkanDebug::ErrorCheck( CreateInstance("Vulkan App") );

	VulkanDebug::InitDebug(instance);

	// Create VkDevice
	VulkanDebug::ErrorCheck( CreateDevice() );

	// Get the graphics queue
	vkGetDeviceQueue(device, 0, 0, &queue);	// Note that queueFamilyIndex is hard coded to 0

	// Gather physical device memory properties
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// Setup function pointers for the swap chain
	swapChain.connect(instance, physicalDevice, device);

	// Synchronization code missing here, VkSemaphore etc.
}

VulkanBase::~VulkanBase()
{
	swapChain.cleanup();

	vkDestroyCommandPool(device, commandPool, nullptr);

	// Cleanup depth stencil data
	vkDestroyImageView(device, depthStencil.view, nullptr);
	vkDestroyImage(device, depthStencil.image, nullptr);
	vkFreeMemory(device, depthStencil.memory, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
	}

	vkDestroyDevice(device, nullptr);

	VulkanDebug::CleanupDebugging(instance);

	vkDestroyInstance(instance, nullptr);
}

VkResult VulkanBase::CreateInstance(const char* appName)
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

VkResult VulkanBase::CreateDevice()
{
	// Query for the number of GPUs
	uint32_t gpuCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);

	if (result != VK_SUCCESS)
		VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices failed");

	if (gpuCount < 1)
		VulkanDebug::ConsolePrint("vkEnumeratePhysicalDevices didn't find any valid devices for Vulkan");

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
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; 
	queueInfo.pNext = nullptr;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = 0; // 0 seems to always be the first valid queue (see above)
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

void VulkanBase::CreateCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = 0;									// NOTE: TODO: Need to store this as a member (Use Swapchain)!!!!!
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult r = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
	assert(!r);
}

void VulkanBase::CreateSetupCommandBuffer()
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkResult r = vkAllocateCommandBuffers(device, &allocateInfo, &setupCmdBuffer);
	assert(!r);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = Default is OK, change this if multiple command buffers (primary & secondary)

	// Begin recording commands to the command buffer
	r = vkBeginCommandBuffer(setupCmdBuffer, &beginInfo);
	assert(!r);
}

void VulkanBase::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkResult r = vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);
	assert(!r);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = Default is OK, change this if multiple command buffers (primary & secondary)

	// Begin recording commands to the command buffer
	r = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	assert(!r);
}

void VulkanBase::SetupDepthStencil()
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.format = depthFormat;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent = { windowWidth, windowHeight, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	// The rest can have their default 0 value

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = depthFormat;
	viewCreateInfo.subresourceRange = {};
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	VkResult res = vkCreateImage(device, &imageCreateInfo, nullptr, &depthStencil.image);
	assert(!res);

	// Get memory requirements
	VkMemoryRequirements memRequirments;
	vkGetImageMemoryRequirements(device, depthStencil.image, &memRequirments);
	allocateInfo.allocationSize = memRequirments.size;
	GetMemoryType(memRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocateInfo.memoryTypeIndex);

	res = vkAllocateMemory(device, &allocateInfo, nullptr, &depthStencil.memory);
	assert(!res);

	res = vkBindImageMemory(device, depthStencil.image, depthStencil.memory, 0);
	assert(!res);

	vkTools::setImageLayout(commandBuffer, depthStencil.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	viewCreateInfo.image = depthStencil.image;	// Connect the view with the image
	res = vkCreateImageView(device, &viewCreateInfo, nullptr, &depthStencil.view);
	assert(!res);
}

void VulkanBase::SetupRenderPass()
{
	// Subpass creation, standard code 100% from Vulkan samples
	// Also specifices which attachments from pSubpasses to use
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;
	
	// Attachments creation, standard code 100% from Vulkan samples
	// Basically creates one color attachment and one depth stencil attachment
	VkAttachmentDescription attachments[2];
	attachments[0].format = colorformat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 2;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.pAttachments = attachments;

	VkResult res = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
	assert(!res);
}

void VulkanBase::SetupFrameBuffer()
{
	// The code here depends on the depth stencil buffer, the render pass and the swap chain

	VkImageView attachments[2];
	attachments[1] = depthStencil.view;

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachments;
	createInfo.width = windowWidth;
	createInfo.height = windowHeight;
	createInfo.layers = 1;

	// Create a frame buffer for each swap chain image
	frameBuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		attachments[0] = swapChain.buffers[i].view;
		VkResult res = vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffers[i]);
		assert(!res);
	}
}

void VulkanBase::SetupSwapchain()
{
	// Note that we use the same command buffer for everything right now!
	swapChain.create(commandBuffer, &windowWidth, &windowHeight);
}

void VulkanBase::Prepare()
{
	CreateCommandPool();
	//CreateSetupCommandBuffer();
	CreateCommandBuffer();
	SetupSwapchain();	// Also starts recording to the command buffer with vkBeginCommandBuffer
	SetupDepthStencil();
	SetupRenderPass();
	SetupFrameBuffer();

	// Create command buffers for drawing
	// Render pass
	// Depth stencil
	// etc...


	// Do a lot of setup stuff
	// ...

	// Call vkEndCommandBuffer() on the setupCommandBuffer and then submit it to the VkQueue







	// This code will move to FlushSetupCommandBuffer()
	/*VkResult err = vkEndCommandBuffer(setupCmdBuffer);
	assert(!err);

	// Testing fences
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &setupCmdBuffer;
		submitInfo.commandBufferCount = 1;

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence;
		err = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
		assert(!err);

		err = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(!err);

		err = vkWaitForFences(device, 1, &fence, true, UINT64_MAX);	// Note that VK_TIMEOUT don't count as an error
		vkDestroyFence(device, fence, nullptr);
	}

	// Testing semaphores
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		//semaphoreCreateInfo.flags
			
		VkSemaphore semaphore;
		err = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

		// To test fences we need two command buffer submits to the queue
		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &setupCmdBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphore;
		
		
	}

	// Wait for the queue to be idle again (very ineffective)
 	//err = vkQueueWaitIdle(queue);
 	//assert(!err);

	vkFreeCommandBuffers(device, commandPool, 1, &setupCmdBuffer);
	setupCmdBuffer = VK_NULL_HANDLE;*/
}

void VulkanBase::InitSwapchain()
{
	// Platform dependent code to initialize the window surface
#if defined(_WIN32)
	swapChain.initSurface(windowInstance, window);
#endif
}

void VulkanBase::RenderLoop()
{
	MSG message;

	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void VulkanBase::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		PostQuitMessage(0);
		break;
	}
}

// Creates a window that Vulkan can use for rendering
HWND VulkanBase::CreateWin32Window(HINSTANCE hInstance, WNDPROC WndProc)
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

// Code from Vulkan samples and SaschaWillems
VkBool32 VulkanBase::GetMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}