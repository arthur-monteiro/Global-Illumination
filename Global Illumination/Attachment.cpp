#include "Attachment.h"

Attachment::~Attachment()
{
}

void Attachment::initialize(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout finalLayout, VkAttachmentStoreOp storeOperation, VkImageUsageFlagBits usageType)
{
	m_format = format;
	m_sampleCount = sampleCount;
	m_finalLayout = finalLayout;
	m_storeOperation = storeOperation;
	m_usageType = usageType;
}
