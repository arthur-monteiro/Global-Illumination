#pragma once

#include "VulkanHelper.h"
#include "Image.h"

const int OPERATION_TYPE_UNDEFINED = 0x0;
const int OPERATION_TYPE_COPY_IMAGE = 0x1;
const int OPERATION_TYPE_TRANSIT_IMAGE_LAYOUT = 0x2;

class Operation
{
public:
	Operation() = default;
	~Operation();

	void copyImage(Image* srcImage, VkImageLayout sourceLayout, Image* dstImage, VkImageLayout destinationLayout);
	void transitImageLayout(Image* image, VkImageLayout oldLayout, VkImageLayout newLayout,
	        VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

// Getters
public:
    int getOperationType() { return m_operationType; }
    Image* getSourceImage() { return m_srcImage; }
    Image* getDestinationImage() { return m_dstImage; }
    VkImageLayout getSourceLayout() { return m_srcLayout; }
    VkImageLayout getDestinationLayout() { return m_dstLayout; }
    VkPipelineStageFlags getSourceStage() { return m_sourceStage; }
    VkPipelineStageFlags getDestinationStage() { return m_destinationStage; }

private:
    int m_operationType = OPERATION_TYPE_UNDEFINED;

    Image* m_srcImage;
    Image* m_dstImage;
    VkImageLayout m_srcLayout;
    VkImageLayout m_dstLayout;
    VkPipelineStageFlags m_sourceStage;
    VkPipelineStageFlags m_destinationStage;
};

