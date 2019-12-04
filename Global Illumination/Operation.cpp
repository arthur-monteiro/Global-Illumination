#include "Operation.h"

Operation::~Operation()
{
}

void Operation::copyImage(Image* srcImage, VkImageLayout sourceLayout, Image* dstImage, VkImageLayout destinationLayout)
{
    m_operationType = OPERATION_TYPE_COPY_IMAGE;

    m_srcImage = srcImage;
    m_srcLayout = sourceLayout;
    m_dstImage = dstImage;
    m_dstLayout = destinationLayout;
}

void Operation::transitImageLayout(Image *image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
    m_operationType = OPERATION_TYPE_TRANSIT_IMAGE_LAYOUT;

    m_srcImage = image;
    m_srcLayout = oldLayout;
    m_dstLayout = newLayout;
    m_sourceStage = sourceStage;
    m_destinationStage = destinationStage;
}
