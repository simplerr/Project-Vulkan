#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "VulkanApp.h"
#include "Window.h"

// The Vulkan application
VulkanLib::VulkanApp vulkanApp;

#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	vulkanApp.HandleMessages(hwnd, msg, wParam, lParam);

	// Call default window procedure
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#elif defined(__linux__)
static void handleEvent(const xcb_generic_event_t *event)
{
	// TODO
}
#endif

#if defined(_WIN32)
// Windows entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#elif defined(__linux__)
// Linux entry point
int main(const int argc, const char *argv[])
#endif
{
	VulkanLib::Window window = VulkanLib::Window(1280, 1024);

#if defined(_WIN32)			// Win32
	window.SetupWindow(hInstance, WndProc);
#elif defined(__linux__)	// Linux
	window.SetupWindow();
#endif

	vulkanApp.InitSwapchain(&window);
	vulkanApp.Prepare();
	vulkanApp.RenderLoop();

	return 0;
}