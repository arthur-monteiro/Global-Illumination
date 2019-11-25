#pragma once

#include "Mesh.h"

class Model
{
public:
	virtual ~Model();

	virtual std::vector<VertexBuffer> getVertexBuffers() { return {}; }

	virtual void cleanup(VkDevice device) {}

private:
    
};