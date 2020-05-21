#include "sys.h"


// Commands
VkCommandBuffer VulkanRender::allocateGraphicsCB()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->graphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer temp;
	vkAllocateCommandBuffers(this->device, &allocInfo, &temp);

	return temp;
}

VkCommandBuffer VulkanRender::clearGraphicsCB(const glm::vec3& color)
{
	VkResult r;
	VkCommandBuffer temp = this->allocateGraphicsCB();

	VkClearValue value = {};
	value.color = {
		color.r,
		color.g,
		color.b,
		1.0f
	};

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkRenderPassBeginInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = this->clearRenderPass;
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = this->swapChainExtent;
	rp.clearValueCount = 1;
	rp.pClearValues = &value;
	rp.framebuffer = this->clearFrameBuffer[this->swapChainIndex];

	vkBeginCommandBuffer(temp, &beginInfo);
	vkCmdBeginRenderPass(temp, &rp, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(temp);
	r = vkEndCommandBuffer(temp);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Failed to kill command buffer...");
	}

	return temp;
}

VkCommandBuffer VulkanRender::updateTransformCB()
{
	VkResult r;
	VkCommandBuffer temp = this->allocateGraphicsCB();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(temp, &beginInfo);

	vkCmdUpdateBuffer(
		temp,
		this->uniformBuffer,
		0,
		sizeof(UniformVS),
		&this->uniformVS
	);


	r = vkEndCommandBuffer(temp);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("failed to kill update transform CB");
	}
	return temp;
}

VkCommandBuffer VulkanRender::drawCommand(
	VkBuffer* vertexBuffer,
	VkBuffer* indexBuffer,
	size_t count
)
{
	VkResult r;
	
	VkDeviceSize deviceOffset[] = { 0 };
	
	VkCommandBuffer temp = this->allocateGraphicsCB();
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	VkRenderPassBeginInfo renderPassBegin = {};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.renderPass = this->drawRenderPass;
	renderPassBegin.renderArea.offset = { 0, 0 };
	renderPassBegin.renderArea.extent = this->swapChainExtent;
	renderPassBegin.framebuffer = this->clearFrameBuffer[this->swapChainIndex];
	
	std::vector<VkDescriptorSet> sets = {
		this->uniformDescSet
	};
	
	vkBeginCommandBuffer(temp, &beginInfo);
	
	vkCmdBeginRenderPass(temp, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	
	vkCmdBindPipeline(
		temp,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		this->graphicsPipeline
	);

	vkCmdBindDescriptorSets(
		temp,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		this->pipelineLayout,
		0,
		sets.size(),
		sets.data(),
		0,
		nullptr
	);

	vkCmdBindVertexBuffers(temp, 0, 1, vertexBuffer, deviceOffset);
	vkCmdBindIndexBuffer(temp, *indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(temp, count, 1, 0, 0, 0);
	
	vkCmdEndRenderPass(temp);
	
	r = vkEndCommandBuffer(temp);
	
	if (r != VK_SUCCESS)
	{
		std::runtime_error("failed to kill draw command CB");
	}
	
	return temp;
}

VkCommandBuffer VulkanRender::allocateTransferCB()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->transferCommandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer temp;
	vkAllocateCommandBuffers(this->device, &allocInfo, &temp);
	return temp;
}

void VulkanRender::freeTransferCB(VkCommandBuffer buffer)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);
	vkFreeCommandBuffers(this->device, this->transferCommandPool, 1, &buffer);
}

void VulkanRender::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer temp = this->allocateTransferCB();
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(temp, &beginInfo);
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(temp, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(temp);
	this->freeTransferCB(temp);
}

