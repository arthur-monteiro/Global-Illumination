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
	std::vector<Texture*> getTextures(int meshID);
	glm::mat4 getTransformation() { return m_transformationMatrix; }

private:
    std::vector<MeshPBR> m_meshes;
	glm::mat4 m_transformationMatrix = glm::mat4(1.0f);

private:
    static std::string getTexName(std::string texName, std::string folder);
};