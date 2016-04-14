#include "ModelLoader.h"
#include "StaticModel.h"
#include "VulkanBase.h"

#include <vector>

// TODO: Note that the format should be #include <assimp/Importer.hpp> but something in the project settings is wrong
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/cimport.h"
#include "../external/assimp/assimp/material.h"
#include "../external/assimp/assimp/ai_assert.h"
#include "../external/assimp/assimp/postprocess.h"
#include "../external/assimp/assimp/scene.h"

void ModelLoader::CleanupModels(VkDevice device)
{
	for(auto& model : mModelMap)
	{
		// Free vertex and index buffers
		vkDestroyBuffer(device, model.second->vertices.buffer, nullptr);
		vkFreeMemory(device, model.second->vertices.memory, nullptr);
		vkDestroyBuffer(device, model.second->indices.buffer, nullptr);
		vkFreeMemory(device, model.second->indices.memory, nullptr);

		// Free the texture (NOTE: not sure if this is the right place to delete them, texture loader maybe?)
		if (model.second->texture != nullptr)
		{
			vkDestroyImageView(device, model.second->texture->view, nullptr);		// NOTE: Ugly
			vkDestroyImage(device, model.second->texture->image, nullptr);
			vkDestroySampler(device, model.second->texture->sampler, nullptr);
			vkFreeMemory(device, model.second->texture->deviceMemory, nullptr);
		}

		delete model.second;
	}
}

StaticModel * ModelLoader::LoadModel(VulkanBase* vulkanBase, std::string filename)
{
	// Check if the model already is loaded
	if (mModelMap.find(filename) != mModelMap.end())
		return mModelMap[filename];

	StaticModel* model = nullptr;
	Assimp::Importer importer;

	
	// Load scene from the file.
	const aiScene* scene = importer.ReadFile(filename, aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

	if (scene != nullptr)
	{
		model = new StaticModel;

		// Loop over all meshes
		for (int meshId = 0; meshId < scene->mNumMeshes; meshId++)
		{
			aiMesh* assimpMesh = scene->mMeshes[meshId];

			// Get the diffuse color
			aiColor3D color(0.f, 0.f, 0.f);
			scene->mMaterials[assimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

			Mesh mesh;

			// Load vertices
			for (int vertexId = 0; vertexId < assimpMesh->mNumVertices; vertexId++)
			{
				aiVector3D v = assimpMesh->mVertices[vertexId];
				aiVector3D n = assimpMesh->mNormals[vertexId];
				aiVector3D t = aiVector3D(0, 0, 0);

				if (assimpMesh->HasTextureCoords(0))
					t = assimpMesh->mTextureCoords[0][vertexId];

				n = n.Normalize();
				Vertex vertex(v.x, v.y, v.z, n.x, n.y, n.z, 0, 0, 0, t.x, t.y, color.r, color.g, color.b);

				mesh.vertices.push_back(vertex);
			}

			// Load indices
			for (int faceId = 0; faceId < assimpMesh->mNumFaces; faceId++)
			{
				for (int indexId = 0; indexId < assimpMesh->mFaces[faceId].mNumIndices; indexId++)
					mesh.indices.push_back(assimpMesh->mFaces[faceId].mIndices[indexId]);
			}
			
			model->AddMesh(mesh);
		}

		// Add the model to the model map
		model->BuildBuffers(vulkanBase);		// Build the models buffers here
		mModelMap[filename] = model;
	}
	else {
		// Loading of model failed
		assert(scene);		
	}

	return model;
}
