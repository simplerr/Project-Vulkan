#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "VulkanApp.h"
#include "Window.h"

// The Vulkan application
VulkanApp vulkanApp;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	vulkanApp.HandleMessages(hwnd, msg, wParam, lParam);

	// Call default window procedure
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	Window window = Window(1280, 1024);

#if defined(_WIN32)
	window.SetupWindow(hInstance, WndProc);
#elif defined(__linux__)
	window.SetupWindow();
#endif
	vulkanApp.InitSwapchain(&window);
	vulkanApp.Prepare();
	vulkanApp.RenderLoop();

	return 0;
}