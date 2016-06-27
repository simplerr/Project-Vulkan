#include "Game.h"
#include "Renderer.h"
#include "Window.h"
#include <string>
#include <sstream>

namespace VulkanLib
{

	Game::Game(Renderer* renderer, Window* window)
	{
		mRenderer = renderer;
		mWindow = window;
	}

	Game::~Game()
	{
		mTimer.PrintLog("benchmark.txt");
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

			mRenderer->Update();
			mRenderer->Render();

			// Frame end
			auto fps = mTimer.FrameEnd();

			// Only display fps when 1.0s have passed
			if (fps != -1)
			{
				std::stringstream ss;
				ss << "Project Vulkan [" << fps << "] fps [" << mRenderer->GetNumVertices() << " vertices] [" << mRenderer->GetNumTriangles() << " triangles] [" << mRenderer->GetNumObjects() << " objects]";
				std::string windowTitle = ss.str();//"Project Vulkan: " + std::to_string(fps) + " fps [" + mRenderer->GetNumVertices() + " vertices] [" + mRenderer->GetNumTriangles() + " triangles] [" + mRenderer->GetNumObjects() + " objects]";
				SetWindowText(mWindow->GetHwnd(), windowTitle.c_str());
			}
		}
	}
#endif

	void Game::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
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