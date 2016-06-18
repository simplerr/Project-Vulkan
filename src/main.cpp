#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "VulkanApp.h"
#include "Window.h"
#include "Renderer.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "Game.h"

// The Vulkan application
//VulkanLib::VulkanApp vulkanApp;

VulkanLib::Game* gGame = nullptr;

#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(gGame != nullptr)
		gGame->HandleMessages(hwnd, msg, wParam, lParam);

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
	/*
		Create the window
	*/
	VulkanLib::Window window = VulkanLib::Window(1280, 1024);

#if defined(_WIN32)			// Win32
	window.SetupWindow(hInstance, WndProc);
#elif defined(__linux__)	// Linux
	window.SetupWindow();
#endif

	// Create the renderer	
	VulkanLib::Renderer* renderer = new VulkanLib::VulkanRenderer(&window);
	//VulkanLib::Renderer* renderer = new VulkanLib::OpenGLRenderer(&window);		//

	// Create the game
	gGame = new VulkanLib::Game(renderer, &window);

	// Game loop
	gGame->RenderLoop();

	delete renderer;
	delete gGame;

	return 0;
}