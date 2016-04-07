#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "VulkanApp.h"

// https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkApplicationInfo


VulkanApp* vulkanApp = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Call default window procedure
	if (vulkanApp != nullptr)
		vulkanApp->HandleMessages(hwnd, msg, wParam, lParam);

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	vulkanApp = new VulkanApp();
	vulkanApp->CreateWin32Window(hInstance, WndProc);
	//vulkanApp->initSwapchain();
	//vulkanApp->setupSwapchain(); // The command buffers needs to be created before the swapchain
	//vulkanApp->renderLoop();
	/*
	vulkanApp->setupWindow(hInstance, WndProc);

	
	vulkanApp->prepare();
	*/

	delete vulkanApp;

	return 0;
}