#include "VulkanRenderer.h"
#include "VulkanApp.h"

namespace VulkanLib
{
	void VulkanRenderer::Init(Window* window)
	{
		mVulkanApp.InitSwapchain(window);
		mVulkanApp.Prepare();
		mVulkanApp.RenderLoop();
	}

	void VulkanRenderer::SetNumThreads()
	{

	}

	void VulkanRenderer::Render()
	{

	}

	void VulkanRenderer::Update()
	{

	}

	void VulkanRenderer::AddModel(StaticModel* model)
	{

	}

	void VulkanRenderer::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mVulkanApp.HandleMessages(hWnd, uMsg, wParam, lParam);
	}
}