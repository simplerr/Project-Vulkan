#include "Game.h"
#include "Renderer.h"
#include "Window.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "Camera.h"
#include "Object.h"
#include <string>
#include <sstream>
#include <fstream>

namespace VulkanLib
{

	Game::Game(Renderer* renderer, Window* window)
	{
		mRenderer = renderer;
		mWindow = window;
	}

	Game::Game(Window* window)
	{
		mRenderer = nullptr;
		mWindow = window;
	}

	Game::~Game()
	{
		std::ofstream fout;
		fout.open("benchmark.txt", std::fstream::out | std::ofstream::app);

		// Print scene information
		fout << mRenderer->GetName() << "[" << mRenderer->GetNumVertices() << " vertices] [" << mRenderer->GetNumTriangles() << " triangles] [" << mRenderer->GetNumObjects() << " objects]" << std::endl;
		fout << "Threads: " << mRenderer->GetNumThreads() << std::endl;

		// Print benchmark to file
		mTimer.PrintLog(fout);

		fout << "-----------------------------------------------" << std::endl << std::endl;

		fout.close();

		delete mRenderer;
	}

	void Game::InitScene()
	{
		// Create the camera
		VulkanLib::Camera* camera = new VulkanLib::Camera(glm::vec3(500, 1300, 500), 60.0f, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 0.1f, 25600.0f);
		camera->LookAt(glm::vec3(0, 0, 0));
		mRenderer->SetCamera(camera);

		// Add objects
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

					mRenderer->AddObject(object);
				}
			}
		}
	}

#if defined(_WIN32)
	void Game::RenderLoop()
	{
		MSG msg;

		while (true)
		{
			// Frame begin
			mTimer.FrameBegin();

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

			if (mRenderer != nullptr)
			{
				mRenderer->Update();
				mRenderer->Render();

				// Frame end
				auto fps = mTimer.FrameEnd();

				// Only display fps when 1.0s have passed
				if (fps != -1)
				{
					std::stringstream ss;
					ss << "Project Vulkan: " << mRenderer->GetName() << " [" << fps << " fps] [" << mRenderer->GetNumVertices() << " vertices] [" << mRenderer->GetNumTriangles() << " triangles] [" << mRenderer->GetNumObjects() << " objects]";
					ss << " [" << mRenderer->GetNumThreads() << " threads]";
					std::string windowTitle = ss.str();
					SetWindowText(mWindow->GetHwnd(), windowTitle.c_str());
				}
			}
			else if (mRenderer == nullptr)
			{
				// Let the user choose the renderer and # threads
				if (GetAsyncKeyState('1'))
					mRenderer = new VulkanLib::VulkanRenderer(mWindow, 1);
				else if (GetAsyncKeyState('2'))
					mRenderer = new VulkanLib::VulkanRenderer(mWindow, 2);
				else if (GetAsyncKeyState('3'))
					mRenderer = new VulkanLib::VulkanRenderer(mWindow, 3);
				else if (GetAsyncKeyState('4'))
					mRenderer = new VulkanLib::VulkanRenderer(mWindow, 4);
				else if (GetAsyncKeyState('5'))
					mRenderer = new VulkanLib::OpenGLRenderer(mWindow);

				// Init the scene [NOTE][HACK] 
				if (mRenderer != nullptr)
					InitScene();
			}
		}
	}
#endif

	void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(mRenderer != nullptr)
			mRenderer->HandleMessages(hWnd, uMsg, wParam, lParam);

		switch (uMsg)
		{
		case WM_CLOSE:
			DestroyWindow(mWindow->GetHwnd());
			PostQuitMessage(0);
			break;
		}
	}
}