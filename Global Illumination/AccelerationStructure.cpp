#include "AccelerationStructure.h"

AccelerationStructure::~AccelerationStructure()
{
}

void AccelerationStructure::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances)
{
	createAccelerationStructures(device, physicalDevice, commandPool, graphicsQueue, geometryInstances);
}

void AccelerationStructure::cleanup(VkDevice device)
{
	destroyAccelerationStructure(device, m_topLevelAS);

	for (auto& as : m_bottomLevelAS)
		destroyAccelerationStructure(device, as);
}

AccelerationStructureData AccelerationStructure::createBottomLevelAS(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, std::vector<GeometryInstance> geometryInstances)
{
	nv_helpers_vk::BottomLevelASGenerator bottomLevelAS;

	for (const auto& buffer : geometryInstances)
	{
		// Indexed geometry
		bottomLevelAS.AddVertexBuffer(buffer.vertexBuffer, buffer.vertexOffset, buffer.vertexCount,
			sizeof(VertexPBR), buffer.indexBuffer, buffer.indexOffset,
			buffer.indexCount, VK_NULL_HANDLE, 0);
	}

	AccelerationStructureData buffers;

	// Once the overall size of the geometry is known, we can create the handle
	// for the acceleration structure
	buffers.structure = bottomLevelAS.CreateAccelerationStructure(device, VK_FALSE);
	
	// The AS build requires some scratch space to store temporary information.
	// The amount of scratch memory is dependent on the scene complexity.
	VkDeviceSize scratchSizeInBytes = 0;
	// The final AS also needs to be stored in addition to the existing vertex
	// buffers. It size is also dependent on the scene complexity.
	VkDeviceSize resultSizeInBytes = 0;
	bottomLevelAS.ComputeASBufferSizes(device, buffers.structure, &scratchSizeInBytes,
		&resultSizeInBytes);

	// Once the sizes are obtained, the application is responsible for allocating
	// the necessary buffers. Since the entire generation will be done on the GPU,
	// we can directly allocate those in device local mem
	createBuffer(device, physicalDevice, scratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers.scratchBuffer, buffers.scratchMem);
	createBuffer(device, physicalDevice, resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers.resultBuffer, buffers.resultMem);

	// Build the acceleration structure. Note that this call integrates a barrier
	// on the generated AS, so that it can be used to compute a top-level AS right
	// after this method.
	bottomLevelAS.Generate(device, commandBuffer, buffers.structure, buffers.scratchBuffer,
		0, buffers.resultBuffer, buffers.resultMem, false, VK_NULL_HANDLE);

	return buffers;
}

void AccelerationStructure::createTopLevelAS(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, const std::vector<std::pair<VkAccelerationStructureNV, glm::mat4x4>>& instances)
{
	// Gather all the instances into the builder helper
	for (size_t i = 0; i < instances.size(); i++)
	{
		// For each instance we set its instance index to its index i in the
		// instance vector, and set its hit group index to 2*i. The hit group
		// index defines which entry of the shader binding table will contain the
		// hit group to be executed when hitting this instance. We set this index
		// to i due to the use of 1 type of rays in the scene: the camera rays
		m_topLevelASGenerator.AddInstance(instances[i].first, instances[i].second,
			static_cast<uint32_t>(i), static_cast<uint32_t>(2 * i));
	}

	// Once all instances have been added, we can create the handle for the TLAS
	m_topLevelAS.structure = m_topLevelASGenerator.CreateAccelerationStructure(device, VK_TRUE);

	// As for the bottom-level AS, the building the AS requires some scratch
	// space to store temporary data in addition to the actual AS. In the case
	// of the top-level AS, the instance descriptors also need to be stored in
	// GPU memory. This call outputs the memory requirements for each (scratch,
	// results, instance descriptors) so that the application can allocate the
	// corresponding memory
	VkDeviceSize scratchSizeInBytes, resultSizeInBytes, instanceDescsSizeInBytes;
	m_topLevelASGenerator.ComputeASBufferSizes(device, m_topLevelAS.structure,
		&scratchSizeInBytes, &resultSizeInBytes,
		&instanceDescsSizeInBytes);

	// Create the scratch and result buffers. Since the build is all done on
	// GPU, those can be allocated in device local memory
	createBuffer(device, physicalDevice, scratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_topLevelAS.scratchBuffer, m_topLevelAS.scratchMem);
	createBuffer(device, physicalDevice, resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_topLevelAS.resultBuffer, m_topLevelAS.resultMem);

	// The buffer describing the instances: ID, shader binding information,
	// matrices ... Those will be copied into the buffer by the helper through
	// mapping, so the buffer has to be allocated in host visible memory.
	createBuffer(device, physicalDevice, instanceDescsSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_topLevelAS.instancesBuffer,
		m_topLevelAS.instancesMem);

	// After all the buffers are allocated, or if only an update is required, we
	// can build the acceleration structure. Note that in the case of the update
	 // we also pass the existing AS as the 'previous' AS, so that it can be
	 // refitted in place. Build the acceleration structure. Note that this call
	 // integrates a barrier on the generated AS, so that it can be used to compute
	 // a top-level AS right after this method.
	m_topLevelASGenerator.Generate(device, commandBuffer, m_topLevelAS.structure,
		m_topLevelAS.scratchBuffer, 0, m_topLevelAS.resultBuffer,
		m_topLevelAS.resultMem, m_topLevelAS.instancesBuffer,
		m_topLevelAS.instancesMem, VK_FALSE, VK_NULL_HANDLE);
}

void AccelerationStructure::createAccelerationStructures(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances)
{
	// Create a one-time command buffer in which the AS build commands will be
	// issued
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	if(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::logic_error("rt vkAllocateCommandBuffers failed");
	}

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// For each geometric object, we compute the corresponding bottom-level
	// acceleration structure (BLAS)
	m_bottomLevelAS.resize(geometryInstances.size());

	std::vector<std::pair<VkAccelerationStructureNV, glm::mat4x4>> instances;

	for (size_t i = 0; i < geometryInstances.size(); i++)
	{
		m_bottomLevelAS[i] = createBottomLevelAS(
			device,
			physicalDevice,
			commandBuffer, 
			{
				geometryInstances[i]
			});
		instances.push_back({ m_bottomLevelAS[i].structure, geometryInstances[i].transform });
	}

	// Create the top-level AS from the previously computed BLAS
	createTopLevelAS(device, physicalDevice, commandBuffer, instances);

	// End the command buffer and submit it
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void AccelerationStructure::destroyAccelerationStructure(VkDevice device, const AccelerationStructureData& as)
{
	vkDestroyBuffer(device, as.scratchBuffer, nullptr);
	vkFreeMemory(device, as.scratchMem, nullptr);
	vkDestroyBuffer(device, as.resultBuffer, nullptr);
	vkFreeMemory(device, as.resultMem, nullptr);
	vkDestroyBuffer(device, as.instancesBuffer, nullptr);
	vkFreeMemory(device, as.instancesMem, nullptr);
	//vkDestroyAccelerationStructureNV(device, as.structure, nullptr);
}
