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
	Game::Game(Window* window)
	{
		mRenderer = nullptr;
		mWindow = window;

		// Create the camera
		mCamera = new VulkanLib::Camera(glm::vec3(500, 4700, 500), 60.0f, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 0, 0));

		mPipeline = PipelineEnum::TEXTURED;
	}

	Game::~Game()
	{
		PrintBenchmark();

		delete mRenderer;
		delete mCamera;
	}

	void Game::PrintBenchmark()
	{
		std::ofstream fout;
		fout.open("benchmark.txt", std::fstream::out | std::ofstream::app);

		fout << "Test case: " << mTestCaseName << std::endl;

		// Print scene information
		mRenderer->OutputLog(fout);

		// Print benchmark to file
		mTimer.PrintLog(fout);

		fout << "-----------------------------------------------" << std::endl << std::endl;

		fout.close();
	}

	void Game::InitScene()
	{
		mRenderer->SetCamera(mCamera);

		/*Object* object = new Object(glm::vec3(0, 0, 0));
		object->SetModel("data/models/Crate.obj");
		object->SetColor(glm::vec3(0.0f, 1.0f, 0.0f));
		object->SetId(OBJECT_ID_PROP);
		object->SetRotation(glm::vec3(180, 0, 0));
		object->SetPipeline(mPipeline);
		object->SetScale(glm::vec3(100.0f));

		mRenderer->AddObject(object);*/

		// Change depending on test case
		InitLowDetailTestCase();	// [TODO] OpenGL still gets affected by pipeline state changes here
		//InitPipelineTestCase();

		// Creates the instancing array from all the objects
		mRenderer->Init();
	}

	void Game::InitLowDetailTestCase()
	{
		mTestCaseName = "Low detail";

		// Add objects
		int size = 10;
		int i = 0;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				for (int z = 0; z < size; z++)
				{
					Object* object = new Object(glm::vec3(x * 150, -100 - y * 150, z * 150));
					//object->SetModel("data/models/teapot.3ds");
					object->SetModel("data/models/Crate.obj");
					object->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
					object->SetId(OBJECT_ID_PROP);
					object->SetRotation(glm::vec3(180, 0, 0));
					object->SetScale(glm::vec3(3.0f));
					object->SetPipeline(PipelineEnum::COLORED);

					mRenderer->AddObject(object);

					i++;
				}
			}
		}
	}

	void Game::InitPipelineTestCase()
	{
		mTestCaseName = "Pipeline swapping";

		// Add objects
		int size = 10;
		int i = 0;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				for (int z = 0; z < size; z++)
				{
					Object* object = new Object(glm::vec3(x * 150, -100 - y * 150, z * 150));
					object->SetModel("data/models/Crate.obj");
					object->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
					object->SetId(OBJECT_ID_PROP);
					object->SetRotation(glm::vec3(180, 0, 0));
					object->SetScale(glm::vec3(40.0f));

					// By alternating between different pipelines the efficiency of swapping pipelines can be tested
					// Move this to a seperate TestCase class? [TODO]
					if (i % 2 == 0)
						object->SetPipeline(PipelineEnum::TEXTURED);
					else
						object->SetPipeline(PipelineEnum::COLORED);

					mRenderer->AddObject(object);

					i++;
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

			if (mRenderer != nullptr && QueryRenderInitKeys())
			{
				PrintBenchmark();
				mTimer.ResetLifetimeCounter();
				delete mRenderer;
			}

			// Let the user choose the renderer and # threads
			if (GetAsyncKeyState('1')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 1);
				InitScene();
			}
			else if (GetAsyncKeyState('2')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 2);
				InitScene();
			}
			else if (GetAsyncKeyState('3')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 3);
				InitScene();
			}
			else if (GetAsyncKeyState('4')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 4);
				InitScene();
			}
			else if (GetAsyncKeyState('5')) {
				// Test the OpenGL implementation
				mRenderer = new VulkanLib::OpenGLRenderer(mWindow);
				InitScene();

				// Test the Vulkan implementation
				/*mRenderer = new VulkanLib::VulkanRenderer(mWindow);
				InitScene();*/
			}	
			else if (GetAsyncKeyState('6')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 1, true);
				InitScene();
			}
			else if (GetAsyncKeyState('7')) {
				mRenderer = new VulkanLib::OpenGLRenderer(mWindow, true);
				InitScene();
			}
			else if (GetAsyncKeyState('8')) {
				mRenderer = new VulkanLib::VulkanRenderer(mWindow, 1, false, true);
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

	// 
	//	Helper functions
	//
	bool Game::QueryRenderInitKeys()
	{
		return GetAsyncKeyState('1') || GetAsyncKeyState('2') || GetAsyncKeyState('3') || GetAsyncKeyState('4') || GetAsyncKeyState('5') || GetAsyncKeyState('6') || GetAsyncKeyState('7') || GetAsyncKeyState('8');
	}

	std::string Game::GetPipelineStr()
	{
		std::string pipeline = "COLORED";

		if (mPipeline == PipelineEnum::TEXTURED)
			pipeline = "TEXTURED";

		return pipeline;
	}
}