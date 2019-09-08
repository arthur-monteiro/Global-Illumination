#pragma once

#include "Mesh.h"

class Model
{
public:
	void loadObj(Vulkan* vk, std::string path, std::string mtlFolder);

	void cleanup(VkDevice device);

	std::vector<MeshBase*> getMeshes()
	{
		std::vector<MeshBase*> r;
		for (int i(0); i < m_meshes.size(); ++i)
		{
			if (std::find(m_toBeLast.begin(), m_toBeLast.end(), i) != m_toBeLast.end())
				continue;
			r.push_back(&m_meshes[i]);
		}

		for (int i(0); i < m_toBeLast.size(); ++i)
			r.push_back(&m_meshes[m_toBeLast[i]]);
		
		return r;
	}

private:
	std::string getTexName(std::string texName, std::string folder)
	{
		return texName != "" ? folder + "/" + texName : "Textures/white_pixel.jpg";
	}

private:
	std::vector<MeshPBR> m_meshes;

	std::vector<int> m_toBeLast = { 2 }; // flower contains alpha blending
};

