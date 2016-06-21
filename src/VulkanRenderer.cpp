#include "VulkanRenderer.h"
#include "VulkanApp.h"

namespace VulkanLib
{

	VulkanRenderer::VulkanRenderer(Window* window)
	{
		mVulkanApp = new VulkanApp();

		mVulkanApp->InitSwapchain(window);
		mVulkanApp->Prepare();
		//mVulkanApp.RenderLoop();
	}

	void VulkanRenderer::Cleanup()
	{

	}

	void VulkanRenderer::SetNumThreads()
	{

	}

	void VulkanRenderer::Render()
	{
		mVulkanApp->Render();
	}

	void VulkanRenderer::Update()
	{
		mVulkanApp->Update();
	}

	void VulkanRenderer::AddModel(StaticModel* model)
	{

	}

	void VulkanRenderer::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);
	}
	void VulkanRenderer::SetCamera(Camera * camera)
	{
		mCamera = camera;
		mVulkanApp->SetCamera(mCamera);
	}
}