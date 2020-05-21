#include "sys.h"


// Vertices Buffer
void VulkanRender::createVerticesBuffer()
{
	// Vertices List
	this->verticesList = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f)
	};
	// Size
	size_t s = sizeof(glm::vec3) * this->verticesList.size();
	// Staging
	this->createBuffer(
		&this->verticesStagingBuffer,
		&this->verticesStagingDeviceMemory,
		s,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// Upload Buffer Data
	this->uploadBufferData(&this->verticesStagingDeviceMemory, this->verticesList.data(), s);
	// Buffer
	this->createBuffer(
		&this->verticesBuffer,
		&this->verticesDeviceMemory,
		s,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	// Copy Staging to Buffer
	this->copyBuffer(this->verticesStagingBuffer, this->verticesBuffer, s);
}

void VulkanRender::destroyVerticesBuffer()
{
	vkFreeMemory(this->device, this->verticesDeviceMemory, nullptr);
	vkDestroyBuffer(this->device, this->verticesBuffer, nullptr);
	vkFreeMemory(this->device, this->verticesStagingDeviceMemory, nullptr);
	vkDestroyBuffer(this->device, this->verticesStagingBuffer, nullptr);
}