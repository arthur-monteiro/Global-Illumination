#include "Model.h"

#include <tiny_obj_loader.h>

void Model::loadObj(Vulkan* vk, std::string path, std::string mtlFolder)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err, warn;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), mtlFolder.c_str()))
		throw std::runtime_error(err);

#ifndef NDEBUG
	if (err != "")
		std::cout << "[Chargement objet] Erreur : " << err << " pour " << path << " !" << std::endl;
	if (warn != "")
		std::cout << "[Chargement objet]  Attention : " << warn << " pour " << path << " !" << std::endl;
#endif // !NDEBUG

	m_meshes.resize(materials.size());

	std::vector<std::unordered_map<VertexPBR, uint32_t>> uniqueVertices = {}; 
	uniqueVertices.resize(materials.size());
	std::vector<std::vector<VertexPBR>> vertices;
	vertices.resize(materials.size());
	std::vector<std::vector<uint32_t>> indices;
	indices.resize(materials.size());

	for (const auto& shape : shapes)
	{
		int numVertex = 0;
		for (const auto& index : shape.mesh.indices)
		{
			VertexPBR vertex = {};

			if (shape.mesh.material_ids[numVertex / 3] < 0)
				continue;

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal =
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			if (uniqueVertices[shape.mesh.material_ids[numVertex / 3]].count(vertex) == 0)
			{
				uniqueVertices[shape.mesh.material_ids[numVertex / 3]][vertex] = static_cast<uint32_t>(vertices[shape.mesh.material_ids[numVertex / 3]].size());
				vertices[shape.mesh.material_ids[numVertex / 3]].push_back(vertex);
			}

			indices[shape.mesh.material_ids[numVertex / 3]].push_back(uniqueVertices[shape.mesh.material_ids[numVertex / 3]][vertex]);

			numVertex++;
		}
	}

	std::array<VertexPBR, 3> tempTriangle;
	for (int k(0); k < indices.size(); ++k)
	{
		for (int i(0); i <= indices[k].size(); ++i)
		{
			if (i != 0 && i % 3 == 0)
			{
				glm::vec3 edge1 = tempTriangle[1].pos - tempTriangle[0].pos;
				glm::vec3 edge2 = tempTriangle[2].pos - tempTriangle[0].pos;
				glm::vec2 deltaUV1 = tempTriangle[1].texCoord - tempTriangle[0].texCoord;
				glm::vec2 deltaUV2 = tempTriangle[2].texCoord - tempTriangle[0].texCoord;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 tangent;
				tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
				tangent = glm::normalize(tangent);

				for (int j(i - 1); j > i - 4; --j)
					vertices[k][indices[k][j]].tangent = tangent;
			}

			if (i == indices[k].size())
				break;

			tempTriangle[i % 3] = vertices[k][indices[k][i]];
		}
	}

	for (int i(0); i < vertices.size(); ++i)
	{
		m_meshes[i].loadVertices(vk, vertices[i], indices[i]);
		//m_meshes[i].loadTextureFromFile(vk, { getTexName(materials[i].diffuse_texname, mtlFolder) , getTexName(materials[i].bump_texname, mtlFolder),
		//	getTexName(materials[i].specular_highlight_texname, mtlFolder), getTexName(materials[i].ambient_texname, mtlFolder), 
		//	getTexName(materials[i].ambient_texname, mtlFolder) });
		m_meshes[i].createTextureSampler(vk, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		m_meshes[i].clearVertices();
	}
}

void Model::cleanup(VkDevice device)
{
	for (int i(0); i < m_meshes.size(); ++i)
		m_meshes[i].cleanup(device);
}
