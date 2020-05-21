#include "sys.h"

VulkanRender* VulkanRender::_inst = nullptr;

VulkanRender* VulkanRender::getInstance()
{
	if (_inst == nullptr)
	{
		_inst = new VulkanRender;
	}

	return _inst;
}

void VulkanRender::destroy()
{
	_inst->release();

	delete _inst;
}


void VulkanRender::init(bool useLayer)
{
	this->useLayer = useLayer;

	createInstance();
	if (useLayer)
	{
		createDebugReport();
	}
	createSurface();
	queryPhysicalDevice();
	createDevice();
	createSwapChain();
	createSwapChainImageViews();
	createCommandPools();
	createRenderPass();
	createFrameBuffer();
	createSemaphore();
	createFence();

	this->createDescPool();

	this->createVerticesBuffer();
	this->createIndexBuffer();
	this->createUniformBuffer();

	this->createGraphicsPipeline();
}

void VulkanRender::release()
{
	this->destroyGraphicsPipeline();

	this->destroyUniformBuffer();
	this->destroyIndexBuffer();
	this->destroyVerticesBuffer();

	this->destroyDescPool();

	vkDestroyFence(device, inFlight, nullptr);

	vkDestroySemaphore(device, renderFinish, nullptr);
	vkDestroySemaphore(device, imageAvailable, nullptr);

	destroyFrameBuffer(drawFrameBuffer);
	destroyFrameBuffer(clearFrameBuffer);

	vkDestroyRenderPass(device, drawRenderPass, nullptr);
	vkDestroyRenderPass(device, clearRenderPass, nullptr);

	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

	for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (useLayer)
	{
		destroyDebugReport();
	}

	vkDestroyInstance(instance, nullptr);
}

void VulkanRender::startFrame()
{
	vkAcquireNextImageKHR(
		device,
		swapChain,
		std::numeric_limits<uint64_t>::max(),
		imageAvailable,
		VK_NULL_HANDLE,
		&swapChainIndex
	);

	this->clear(glm::vec3(0.0f));
}

void VulkanRender::endFrame()
{
	VkResult r;

	// Wait for fence
	vkWaitForFences(device, 1, &inFlight, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(device, 1, &inFlight);

	// Submit
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = commandBufferList.size();
	submitInfo.pCommandBuffers = commandBufferList.data();

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinish;
	
	r = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlight);
	//vkQueueWaitIdle(graphicsQueue);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit to graphics queue...");
	}

	// Present
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinish;

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &swapChainIndex;

	r = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit present queue.");
	}

	vkQueueWaitIdle(presentQueue);

	vkFreeCommandBuffers(
		device,
		graphicsCommandPool,
		commandBufferList.size(),
		commandBufferList.data()
	);

	commandBufferList.clear();
}

void VulkanRender::clear(const glm::vec3& clr)
{
	this->addGraphicCommand(this->clearGraphicsCB(clr));
}

void VulkanRender::setProj(const glm::mat4& proj)
{
	uniformVS.proj = proj;
	uniformVS.proj[1][1] *= -1.0f;
}

void VulkanRender::setView(const glm::mat4& view)
{
	uniformVS.view = view;
}

void VulkanRender::setModel(const glm::mat4& model)
{
	uniformVS.model = model;
}

void VulkanRender::updateTransforms()
{
	this->addGraphicCommand(this->updateTransformCB());
}

void VulkanRender::drawQuad()
{
	this->addGraphicCommand(
		this->drawCommand(
			&this->verticesBuffer,
			&this->indexBuffer,
			indexList.size()
		)
	);
}
