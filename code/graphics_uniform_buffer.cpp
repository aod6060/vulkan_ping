#include "sys.h"

// Uniform Buffer
void VulkanRender::createUniformBuffer()
{
	VkResult r;
	// Size
	size_t s = sizeof(UniformVS);
	// Buffer
	// Create Buffer
	this->createBuffer(
		&this->uniformBuffer,
		&this->uniformDeviceMemory,
		s,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// Upload Buffer
	this->uploadBufferData(&this->uniformDeviceMemory, &this->uniformVS, sizeof(UniformVS));
	// uniform desc layout set
	// Vertex Shader UBOs
	VkDescriptorSetLayoutBinding descSetLayoutBinding = {};
	descSetLayoutBinding.binding = 0;
	descSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLayoutBinding.descriptorCount = 1;
	descSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// Vertex Descriptor Create Info
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = &descSetLayoutBinding;
	r = vkCreateDescriptorSetLayout(this->device, &createInfo, nullptr, &this->uniformDescLayoutSet);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("failed to create vertex desc set layout");
	}
	// uniform desc set
	// Uniform Allocation
	VkDescriptorSetAllocateInfo descSetAllocInfo = {};
	descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo.descriptorPool = this->descPool;
	descSetAllocInfo.descriptorSetCount = 1;
	descSetAllocInfo.pSetLayouts = &this->uniformDescLayoutSet;
	r = vkAllocateDescriptorSets(this->device, &descSetAllocInfo, &this->uniformDescSet);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("failed to allocate desc set");
	}
	// Update Desc Sets
	// Descriptor Buffer Info
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformVS);
	// Uniform Write Desc Set
	VkWriteDescriptorSet uniformDescWrite = {};
	uniformDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformDescWrite.dstSet = this->uniformDescSet;
	uniformDescWrite.dstBinding = 0;
	uniformDescWrite.dstArrayElement = 0;
	uniformDescWrite.descriptorCount = 1;
	uniformDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformDescWrite.pBufferInfo = &bufferInfo;
	std::vector<VkWriteDescriptorSet> writes = { uniformDescWrite };
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

void VulkanRender::destroyUniformBuffer()
{
	vkFreeDescriptorSets(this->device, this->descPool, 1, &this->uniformDescSet);
	vkDestroyDescriptorSetLayout(device, this->uniformDescLayoutSet, nullptr);
	vkFreeMemory(this->device, this->uniformDeviceMemory, nullptr);
	vkDestroyBuffer(this->device, this->uniformBuffer, nullptr);
}

void VulkanRender::updateUniformBuffer()
{
	this->uploadBufferData(
		&this->uniformDeviceMemory,
		&this->uniformVS,
		sizeof(UniformVS)
	);
}