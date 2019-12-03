#pragma once

#include "VulkanHelper.h"
#include "Image.h"

struct CopyImageOperation
{
	Image* srcImage;
	Image* dstImage;
};

class Operation
{
public:
	Operation() = default;
	~Operation();

	void addCopyImage(Image* srcImage, Image* dstImage);

// Getters
public:
	std::vector<CopyImageOperation> getCopyImageOperation() { return m_copyImageOperations; }

private:
	std::vector<CopyImageOperation> m_copyImageOperations;
};

