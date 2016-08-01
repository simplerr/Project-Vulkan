#include "VulkanRenderer.h"
#include "VulkanApp.h"
#include "Object.h"
#include "StaticModel.h"

namespace VulkanLib
{

	VulkanRenderer::VulkanRenderer(Window* window, bool useInstancing)
	{
		mVulkanApp = new VulkanApp();

		mVulkanApp->InitSwapchain(window);
		mVulkanApp->Prepare();
		
		//mVulkanApp.RenderLoop();
	}

	VulkanRenderer::VulkanRenderer(Window* window, int numThreads, bool useInstancing, bool useStaticCommandBuffers)
	{
		mVulkanApp = new VulkanApp();

		//mVulkanApp->mTestModel = mModelLoader.LoadModel(mVulkanApp, "data/models/teapot.3ds");
		mVulkanApp->mTestModel = mModelLoader.LoadModel(mVulkanApp, "data/models/Crate.obj");

		mVulkanApp->EnableInstancing(useInstancing);	// [NOTE] The order is important, must be before Prepare()
		mVulkanApp->EnableStaticCommandBuffers(useStaticCommandBuffers);
		mVulkanApp->InitSwapchain(window);
		mVulkanApp->Prepare();
		
		mVulkanApp->SetupMultithreading(numThreads);

		mUseInstancing = useInstancing;
		mUseStaticCommandBuffer = useStaticCommandBuffers;
	}

	void VulkanRenderer::Init()
	{
		mVulkanApp->PrepareInstancing();
		mVulkanApp->RecordStaticCommandBuffers();	// [NOTE] Has to be called after all the objects are added!
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
	void VulkanRenderer::OutputLog(std::ostream & fout)
	{
		fout << GetName() << "\n[" << GetNumVertices() << " vertices] [" << GetNumTriangles() << " triangles] [" << GetNumObjects() << " objects]" << std::endl;
		fout << "Threads: " << GetNumThreads() << std::endl;

		if(mUseInstancing)
			fout << "Pipeline: " << "Instancing" << std::endl;
		else if (mUseStaticCommandBuffer)
			fout << "Pipeline: " << "Static command buffers" << std::endl;
		else
			fout << "Pipeline: " << "Basic" << std::endl;
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

		mNumVertices += model.mesh->GetNumVertics();
		mNumTriangles += model.mesh->GetNumIndices();
		mNumObjects++;
	}

	int VulkanRenderer::GetNumVertices()
	{
		return mNumVertices;
	}

	int VulkanRenderer::GetNumTriangles()
	{
		return mNumVertices / 3;
	}

	int VulkanRenderer::GetNumObjects()
	{
		return mNumObjects;
	}

	std::string VulkanRenderer::GetName()
	{
		return "Vulkan renderer 1.0";
	}

	int VulkanRenderer::GetNumThreads()
	{
		return mVulkanApp->mNumThreads; // OpenGL allways one thread
	}

	Camera* VulkanRenderer::GetCamera()
	{
		return mCamera;
	}
}