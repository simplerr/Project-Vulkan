#pragma once
#include "Renderer.h"
#include "VulkanApp.h"

namespace VulkanLib
{
	class VulkanApp;

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer(Window* window);

		virtual void Cleanup();
		virtual void SetNumThreads();
		virtual void Render();
		virtual void Update();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual void SetCamera(Camera* camera);

	private:
		VulkanApp* mVulkanApp;
		Camera* mCamera;

	};

	

}