#pragma once

#include "VulkanHelper.h"
#include "Image.h"

class Attachment
{
public:
	Attachment() {}
	~Attachment();

	void initialize(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout finalLayout, VkAttachmentStoreOp storeOperation, VkImageUsageFlags usageType);
	void setSampleCount(VkSampleCountFlagBits sampleCount) { m_sampleCount = sampleCount; }

// Getters
public:
	VkFormat getFormat() { return m_format; }
	VkSampleCountFlagBits getSampleCount() { return m_sampleCount; }
	VkAttachmentStoreOp getStoreOperation() { return m_storeOperation; }
	VkImageLayout getFinalLayout() { return m_finalLayout; }
	VkImageUsageFlags getUsageType() { return m_usageType; }
	
private:
	VkExtent2D m_extent;
	VkFormat m_format;
	VkSampleCountFlagBits m_sampleCount;
	VkImageLayout m_finalLayout;
	VkAttachmentStoreOp m_storeOperation;

	VkImageUsageFlags m_usageType;
};

