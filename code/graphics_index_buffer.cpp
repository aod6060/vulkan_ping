#include "sys.h"

// Index Buffer
void VulkanRender::createIndexBuffer()
{
	// Index List
	this->indexList = {
		0, 1, 2, 2, 1, 3
	};

	std::cout << indexList.size() << ", " << indexList.size() * sizeof(uint32_t) << std::endl;

	// Size
	size_t s = sizeof(uint32_t) * this->indexList.size();
	// Staging
	this->createBuffer(
		&this->indexStagingBuffer,
		&this->indexStagingDeviceMemory,
		s,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// Upload Buffer Data
	this->uploadBufferData(&this->indexStagingDeviceMemory, this->indexList.data(), s);
	// Buffer
	this->createBuffer(
		&this->indexBuffer,
		&this->indexDeviceMemory,
		s,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	// Copy
	this->copyBuffer(this->indexStagingBuffer, this->indexBuffer, s);
}

void VulkanRender::destroyIndexBuffer()
{
	vkFreeMemory(this->device, this->indexDeviceMemory, nullptr);
	vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
	vkFreeMemory(this->device, this->indexStagingDeviceMemory, nullptr);
	vkDestroyBuffer(this->device, this->indexStagingBuffer, nullptr);
}