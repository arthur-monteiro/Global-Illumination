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

private:
    std::vector<MeshPBR> m_meshes;

private:
    static std::string getTexName(std::string texName, std::string folder);
};