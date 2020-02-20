#pragma once

#include <unordered_map>
#include <mutex>
#include <array>

#include "Model.h"
#include "Mesh.h"
#include "Texture.h"

struct MeshPBR
{
    Mesh<VertexPBR> mesh;
	std::array<Texture, 5> textures;
};

class ModelPBR : public Model
{
public:
    ModelPBR() = default;
    ~ModelPBR();

    void loadFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::mutex * graphicsQueueMutex,
            std::string filename, std::string mtlFolder);

    void cleanup(VkDevice device);

    std::vector<VertexBuffer> getVertexBuffers();
	std::vector<Image*> getImages(int meshID);
	Sampler* getSampler() { return &m_sampler; }
	glm::mat4 getTransformation() { return m_transformationMatrix; }

private:
    //std::vector<MeshPBR> m_meshes;
	Mesh<VertexPBR> m_mesh;
	std::vector<Image> m_images;
	Sampler m_sampler;
	glm::mat4 m_transformationMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));

	std::vector<int> m_toBeLast = { 2, 19, 0 }; // flower contains alpha blending

private:
    static std::string getTexName(std::string texName, std::string folder);
};