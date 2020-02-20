#pragma once

#include "VulkanHelper.h"

class Semaphore
{
public:
    Semaphore() {}
    ~Semaphore();

    bool initialize(VkDevice device);

	void cleanup(VkDevice device);

// Getters
public:
    VkSemaphore getSemaphore() { return m_semaphore; }
    VkPipelineStageFlags getPipelineStage() { return m_pipelineStage; }

// Setters
public:
    void setPipelineStage(VkPipelineStageFlags pipelineStage) { m_pipelineStage = pipelineStage; }

private:
    VkSemaphore m_semaphore;
    VkPipelineStageFlags m_pipelineStage;
};