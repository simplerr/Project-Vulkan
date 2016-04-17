#if defined(_WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <chrono>

#include "VulkanBase.h"
#include "VulkanDebug.h"
#include "../base/vulkanTextureLoader.hpp"

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

	// Destroy semaphores
	vkDestroySemaphore(device, presentComplete, nullptr);
	vkDestroySemaphore(device, renderComplete, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	delete textureLoader;


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

	for (auto& shaderModule : shaderModules)
	{
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}

	

	vkDestroyDevice(device, nullptr);

	VulkanDebug::CleanupDebugging(instance);

	vkDestroyInstance(instance, nullptr);
}

void VulkanBase::Prepare()
{
	CompileShaders();				// Compile shaders using batch files
	CreateCommandPool();			// Create a command pool to allocate command buffers from
	CreateSetupCommandBuffer();		// Create the setup command buffer used for queuing initialization command, also starts recording to the setup command buffer with vkBeginCommandBuffer
	SetupSwapchain();				// Setup the swap chain with the helper class
	CreateSemaphores();
	CreateCommandBuffers();			// Create the command buffers used for drawing and the image format transitions
	SetupDepthStencil();			// Setup the depth stencil buffer
	SetupRenderPass();				// Setup the render pass
	SetupFrameBuffer();				// Setup the frame buffer, it uses the depth stencil buffer, render pass and swap chain
	ExecuteSetupCommandBuffer();	// Submit all commands so far to the queue, end and free the setup command buffer
	CreateSetupCommandBuffer();		// The derived class will also record initialization commands to the setup command buffer

	// Create a simple texture loader class
	textureLoader = new vkTools::VulkanTextureLoader(physicalDevice, device, queue, commandPool);

	// The derived class initializes:
	// Pipeline
	// Uniform buffers
	// Vertex buffers 
	// Descriptor sets
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

	// Begin recording commands to the setup command buffer
	r = vkBeginCommandBuffer(setupCmdBuffer, &beginInfo);
	assert(!r);
}

void VulkanBase::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// Allocate the command buffers that the image memory barriers will use to change the swap chain image format
	VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(device, &allocateInfo, &prePresentCmdBuffer));
	VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(device, &allocateInfo, &postPresentCmdBuffer));
	
	// Allocate a command buffer for each swap chain image
	renderingCommandBuffers.resize(swapChain.imageCount);
	allocateInfo.commandBufferCount = renderingCommandBuffers.size();

	VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(device, &allocateInfo, renderingCommandBuffers.data()));
}

void VulkanBase::CreateSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VulkanDebug::ErrorCheck(vkCreateSemaphore(device, &createInfo, nullptr, &presentComplete));
	VulkanDebug::ErrorCheck(vkCreateSemaphore(device, &createInfo, nullptr, &renderComplete));
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

	vkTools::setImageLayout(setupCmdBuffer, depthStencil.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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
	// Uses the setup command buffer
	swapChain.create(setupCmdBuffer, &windowWidth, &windowHeight);
}

void VulkanBase::ExecuteSetupCommandBuffer()
{
	if (setupCmdBuffer == VK_NULL_HANDLE)
		return;

	VkResult err = vkEndCommandBuffer(setupCmdBuffer);
	assert(!err);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCmdBuffer;

	err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(!err);

	err = vkQueueWaitIdle(queue);
	assert(!err);

	vkFreeCommandBuffers(device, commandPool, 1, &setupCmdBuffer);
	setupCmdBuffer = VK_NULL_HANDLE;
}

void VulkanBase::SubmitPrePresentMemoryBarrier(VkImage image)
{
	// Copy & paste, I think this can be done smarter (TODO)
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VulkanDebug::ErrorCheck(vkBeginCommandBuffer(prePresentCmdBuffer, &cmdBufInfo));

	VkImageMemoryBarrier prePresentBarrier = vkTools::initializers::imageMemoryBarrier();
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = 0;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;						// New layout for presenting
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	prePresentBarrier.image = image;

	vkCmdPipelineBarrier(
		prePresentCmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_FLAGS_NONE,
		0, nullptr, // No memory barriers,
		0, nullptr, // No buffer barriers,
		1, &prePresentBarrier);

	VulkanDebug::ErrorCheck(vkEndCommandBuffer(prePresentCmdBuffer));

	VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &prePresentCmdBuffer;

	VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void VulkanBase::SubmitPostPresentMemoryBarrier(VkImage image)
{
	// Copy & paste, I think this can be done smarter (TODO)
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VulkanDebug::ErrorCheck(vkBeginCommandBuffer(postPresentCmdBuffer, &cmdBufInfo));

	VkImageMemoryBarrier postPresentBarrier = vkTools::initializers::imageMemoryBarrier();	// TODO: Remove VkTools code
	postPresentBarrier.srcAccessMask = 0;
	postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;				// New layout for rendering
	postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	postPresentBarrier.image = image;

	vkCmdPipelineBarrier(
		postPresentCmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr, // No memory barriers,
		0, nullptr, // No buffer barriers,
		1, &postPresentBarrier);

	VulkanDebug::ErrorCheck(vkEndCommandBuffer(postPresentCmdBuffer));

	VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &postPresentCmdBuffer;

	VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void VulkanBase::InitSwapchain()
{
	// Platform dependent code to initialize the window surface
#if defined(_WIN32)
	swapChain.initSurface(windowInstance, window);
#endif
}

VkPipelineShaderStageCreateInfo VulkanBase::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
#if defined(__ANDROID__)
	shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
	shaderStage.module = vkTools::loadShader(fileName.c_str(), device, stage);		// Uses helper functions (NOTE/TODO)
#endif
	shaderStage.pName = "main";
	assert(shaderStage.module != NULL);
	shaderModules.push_back(shaderStage.module);		// Add them to the vector so they can be cleaned up
	return shaderStage;
}

void VulkanBase::RenderLoop()
{
	MSG msg;

	while (true)
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Render();
		frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		frameTimer = (float)tDiff / 1000.0f;

		// Convert to clamped timer value
		timer += timerSpeed * frameTimer;
		if (timer > 1.0)
		{
			timer -= 1.0f;
		}
		fpsTimer += (float)tDiff;
		if (fpsTimer > 1000.0f)
		{
			std::string windowTitle = "Project Vulkan: " + std::to_string(frameCounter) + " fps";
			SetWindowText(window, windowTitle.c_str());
			fpsTimer = 0.0f;
			frameCounter = 0.0f;
		}
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

VkDevice VulkanBase::GetDevice()
{
	return device;
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