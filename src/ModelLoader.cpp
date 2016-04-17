#include "ModelLoader.h"
#include "StaticModel.h"
#include "VulkanBase.h"
#include "LoadTGA.h"

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

StaticModel* ModelLoader::GenerateTerrain(VulkanBase* vulkanBase, std::string filename)
{
	// Check if the model already is loaded
	if (mModelMap.find(filename) != mModelMap.end())
		return mModelMap[filename];

	// Load the terrain froma .tga file
	TextureData texture;
	LoadTGATextureData((char*)filename.c_str(), &texture);

	StaticModel* terrain = new StaticModel;
	Mesh mesh;

	int vertexCount = texture.width * texture.height;
	int triangleCount = (texture.width - 1) * (texture.height - 1) * 2;
	int x, z;

	mesh.vertices.resize(vertexCount);
	mesh.indices.resize(triangleCount * 3);

	printf("bpp %d\n", texture.bpp);
	for (x = 0; x < texture.width; x++)
		for (z = 0; z < texture.height; z++)
		{
			// Vertex array. You need to scale this properly
			float height = texture.imageData[(x + z * texture.width) * (texture.bpp / 8)] / 20.0f;

			glm::vec3 pos = glm::vec3(x / 1.0, height, z / 1.0);
			glm::vec3 normal = glm::vec3(0, 0, 0);
			glm::vec2 uv = glm::vec2(x / (float)texture.width, z / (float)texture.height);

			Vertex vertex = Vertex(pos, normal, uv, glm::vec3(0, 0, 0), glm::vec3(1.0, 1.0, 1.0));
			mesh.vertices[x + z * texture.width] = vertex;
		}

	// Normal vectors. You need to calculate these.
	for (x = 0; x < texture.width; x++)
	{
		for (z = 0; z < texture.height; z++)
		{
			glm::vec3 p1, p2, p3;
			glm::vec3 edge = { 0.0f, 0.0f, 0.0f };
			int i;

			// p1 [x-1][z-1]
			if (x < 1 && z < 1)
				i = (x + 1 + (z + 1) * texture.width);
			else
				i = (x - 1 + (z - 1) * texture.width);

			// TODO: NOTE: HAX
			if (i < 0)
				i = 0;

			p1 = mesh.vertices[i].Pos;

			// p1 [x-1][z] (if on the edge use [x+1] instead of [x-1])
			if (x < 1)
				i = (x + 1 + (z)* texture.width);
			else
				i = (x - 1 + (z)* texture.width);

			p2 = mesh.vertices[i].Pos;

			// p1 [x][z-1]
			if (z < 1)
				i = (x + (z + 1) * texture.width);
			else
				i = (x + (z - 1) * texture.width);

			p3 = mesh.vertices[i].Pos;

			glm::vec3 e1 = p2 - p1;
			glm::vec3 e2 = p3 - p1;
			glm::vec3 normal = glm::cross(e2, e1);

			if (normal != glm::vec3(0, 0, 0))
				int asda = 1;

			normal = glm::normalize(normal);

			i = (x + z * texture.width);
			mesh.vertices[i].Normal = normal;
		}
	}

	for (x = 0; x < texture.width - 1; x++)
	{
		for (z = 0; z < texture.height - 1; z++)
		{
			// Triangle 1
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 0] = x + z * texture.width;
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 1] = x + (z + 1) * texture.width;
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 2] = x + 1 + z * texture.width;
			// Triangle 2
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 3] = x + 1 + z * texture.width;
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 4] = x + (z + 1) * texture.width;
			mesh.indices[(x + z * (texture.width - 1)) * 6 + 5] = x + 1 + (z + 1) * texture.width;
		}
	}

	terrain->AddMesh(mesh);
	terrain->BuildBuffers(vulkanBase);

	// Add to the map
	mModelMap[filename] = terrain;

	return terrain;
}