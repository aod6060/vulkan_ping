#include "sys.h"

// Buffers
uint32_t VulkanRender::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memProp)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProps);

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & memProp) == memProp)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
	return -1;
}

void VulkanRender::createBuffer(
	VkBuffer* buf,
	VkDeviceMemory* memory,
	size_t size,
	VkBufferUsageFlags usage,
	VkSharingMode sharingMode,
	VkMemoryPropertyFlags memProps
)
{
	VkResult r;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;

	r = vkCreateBuffer(this->device, &bufferInfo, nullptr, buf);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer");
	}

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(device, *buf, &memReq);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memReq.memoryTypeBits,
		memProps);

	r = vkAllocateMemory(device, &allocInfo, nullptr, memory);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(this->device, *buf, *memory, 0);
}

void VulkanRender::uploadBufferData(VkDeviceMemory* memory, void* data, size_t size)
{
	void* d;
	vkMapMemory(this->device, *memory, 0, size, 0, &d);
	memcpy(d, data, size);
	vkUnmapMemory(this->device, *memory);
}
