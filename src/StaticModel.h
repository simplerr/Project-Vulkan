#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan\vulkan.h>

class VulkanBase;

struct Vertex
{
	Vertex() {}
	Vertex(glm::vec3 pos) : Pos(pos) {}
	Vertex(float px, float py, float pz, float nx, float ny, float nz)
		: Pos(px, py, pz), Normal(nx, ny, nz) {}
	Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v, float r, float g, float b)
		: Pos(px, py, pz), Normal(nx, ny, nz), Tangent(tx, ty, tz, 1.0f), Tex(u, v), Color(r, g, b) {}

	glm::vec3 Pos;
	glm::vec3 Normal;
	glm::vec2 Tex;
	glm::vec4 Tangent;
	glm::vec3 Color;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class StaticModel
{
public:
	~StaticModel();

	void AddMesh(Mesh& mesh);

	void BuildBuffers(VulkanBase* vulkanBase);

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} vertices;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} indices;

	int GetNumIndices();

private:
	std::vector<Mesh> mMeshes;
};
