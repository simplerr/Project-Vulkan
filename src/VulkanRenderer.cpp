#include "VulkanRenderer.h"
#include "VulkanApp.h"
#include "Object.h"

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
		// The model loader is responsible for cleaning up the model data
		mModelLoader.CleanupModels(mVulkanApp->GetDevice());
	}

	void VulkanRenderer::SetupMultithreading(int numThreads)
	{
		mVulkanApp->SetupMultithreading(numThreads);
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
	void VulkanRenderer::AddObject(Object* object)
	{
		VulkanModel model;
		model.object = object;

		if(object->GetId() == OBJECT_ID_TERRAIN)
			model.mesh = mModelLoader.GenerateTerrain(mVulkanApp, object->GetModel());
		else
			model.mesh = mModelLoader.LoadModel(mVulkanApp, object->GetModel());

		if (object->GetPipeline() == PipelineEnum::COLORED)
			model.pipeline = mVulkanApp->mPipelines.colored;		// [NOTE][HACK] mPipelines should be private!
		else if (object->GetPipeline() == PipelineEnum::TEXTURED)
			model.pipeline = mVulkanApp->mPipelines.textured;
		else if (object->GetPipeline() == PipelineEnum::STARSPHERE)
			model.pipeline = mVulkanApp->mPipelines.starsphere;

		mVulkanApp->AddModel(model);
	}
}