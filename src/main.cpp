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
#include "Camera.h"
#include "Object.h"

using namespace VulkanLib;

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
	//VulkanLib::Renderer* renderer = new VulkanLib::VulkanRenderer(&window);
	VulkanLib::Renderer* renderer = new VulkanLib::OpenGLRenderer(&window);		//

	renderer->SetupMultithreading(1);

	//renderer->AddModel(new VulkanLib::StaticModel());

	// Create the camera
	VulkanLib::Camera* camera = new VulkanLib::Camera(glm::vec3(500, 1300, 500), 60.0f, (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 25600.0f);
	camera->LookAt(glm::vec3(0, 0, 0));
	renderer->SetCamera(camera);

	// Add starsphere object
	/*Object* sphere = new Object(glm::vec3(0, 0, 0));
	sphere->SetModel("data/models/sphere.obj");
	sphere->SetScale(glm::vec3(100));
	sphere->SetPipeline(PipelineEnum::STARSPHERE);
	sphere->SetId(OBJECT_ID_SKY);
	renderer->AddObject(sphere);*/

 	/*Object* terrain = new Object(glm::vec3(-1000, 0, -1000));
 	terrain->SetModel("data/textures/fft-terrain.tga");
 	terrain->SetPipeline(PipelineEnum::COLORED);
 	terrain->SetScale(glm::vec3(10, 10, 10));
 	terrain->SetColor(glm::vec3(0.0, 0.9, 0.0));
 	terrain->SetId(OBJECT_ID_TERRAIN);
 	renderer->AddObject(terrain);*/

	// Generate some positions
	int size = 10;
	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			for (int z = 0; z < size; z++)
			{
				Object* object = new Object(glm::vec3(x * 150, -100 - y * 150, z * 150));
				object->SetScale(glm::vec3((rand() % 20) / 10.0f));
				object->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
				object->SetId(OBJECT_ID_PROP);

				if (rand() % 2 == 0) {
					object->SetModel("data/models/Cube.obj");
					object->SetRotation(glm::vec3(180, 0, 0));
					object->SetPipeline(PipelineEnum::COLORED);
					//object->SetScale(glm::vec3(500));
				}
				else {
					object->SetModel("data/models/Cube.obj");
					object->SetPipeline(PipelineEnum::COLORED);
					object->SetScale(glm::vec3(5.0f));
				}

				renderer->AddObject(object);
			}
		}
	}

	// Create the game
	gGame = new VulkanLib::Game(renderer, &window);

	// Game loop
	gGame->RenderLoop();

	delete gGame;
	delete renderer;

	return 0;
}