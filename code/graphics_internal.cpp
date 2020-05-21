#include "sys.h"

void VulkanRender::createInstance()
{
	uint32_t version;

	vkEnumerateInstanceVersion(&version);

	if (version == VK_API_VERSION_1_0)
	{
		std::cout << "Using Vulkan 1.0" << std::endl;
	}
	else if (version == VK_API_VERSION_1_1)
	{
		std::cout << "Using Valkan 1.1" << std::endl;
	}
	else
	{
		throw std::runtime_error("Error: Platform doesn't support vulkan...");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Ping";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Ping";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = version;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> layers;
	std::vector<const char*> extensions;
	// Extensions and layers


	if (useLayer)
	{
		layers.push_back("VK_LAYER_LUNARG_standard_validation");
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);


	if (layers.size() > 0)
	{
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
	}

	if (extensions.size() > 0)
	{
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
	}


	VkResult r = vkCreateInstance(&createInfo, nullptr, &instance);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Didn't create instance...");
	}
}

void VulkanRender::createDebugReport()
{
	out.open("log.txt");
	out << "Starting Debuging..." << std::endl;
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	createInfo.pfnCallback = VulkanRender::debugCB;
	createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	createInfo.pUserData = this;

	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	if (func != nullptr)
	{
		VkResult r = func(instance, &createInfo, nullptr, &debugReportCB);

		if (r != VK_SUCCESS)
		{
			throw std::runtime_error("Didn't create debug report");
		}
	}
	else
	{
		throw std::runtime_error("Didn't find PFN_vkCreateDebugReportCallbackEXT");
	}
}

void VulkanRender::destroyDebugReport()
{
	out << "Ending Debuging..." << std::endl;
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	func(instance, debugReportCB, nullptr);
	out.close();
}

VkBool32 VKAPI_CALL VulkanRender::debugCB(
	VkDebugReportFlagsEXT			flags,
	VkDebugReportObjectTypeEXT		objType,
	uint64_t						obj,
	size_t							location,
	int32_t							msgCode,
	const char*						layerPrefix,
	const char*						msg,
	void*							useData
)
{
	VulkanRender* r = (VulkanRender*)useData;

	std::stringstream ss;

	if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) == VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		ss << "Waring) ";
	}
	else if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		ss << "Error) ";
	}
	r->out << ss.str() << layerPrefix << ": " << msg << std::endl;

	r = nullptr;

	return VK_FALSE;
}

void VulkanRender::createSurface()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(app_getWindow(), &info);

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = info.info.win.hinstance;
	createInfo.hwnd = info.info.win.window;

	auto func = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

	if (func != nullptr)
	{
		VkResult r = func(instance, &createInfo, nullptr, &surface);

		if (r != VK_SUCCESS)
		{
			throw std::runtime_error("Surface wasn't created");
		}
	}
	else
	{
		throw std::runtime_error("Did find PFN_vkCreateWin32SurfaceKHR");
	}
}

void VulkanRender::queryPhysicalDevice()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	if (count == 0)
	{
		throw std::runtime_error("Failed to find a GPU capable with Vulkan api...");
	}

	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(instance, &count, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU for Vulkan API!");
	}
}

bool VulkanRender::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete();
}

QueueFamilyIndices VulkanRender::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
	int i = 0;
	for (const auto& family : families)
	{
		// Graphics Family
		if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		// Present Family
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (family.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}
		// Transfer Family
		if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			indices.transferFamily = i;
		}
		if (indices.isComplete())
		{
			break;
		}
		i++;
	}
	indices.printFamilies();
	return indices;
}

void VulkanRender::createDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	std::set<uint32_t> families = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
		indices.transferFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t family : families)
	{
		VkDeviceQueueCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.queueCount = family;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &queuePriority;
		queueInfos.push_back(createInfo);
	}

	VkPhysicalDeviceFeatures features = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.queueCreateInfoCount = queueInfos.size();

	createInfo.pEnabledFeatures = &features;

	std::vector<const char*> layers;
	std::vector<const char*> extensions;

	if (useLayer)
	{
		layers.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (layers.size() > 0)
	{
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
	}

	if (extensions.size() > 0)
	{
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
	}

	VkResult r = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
}

static bool _isTriBool(
	uint32_t v1,
	uint32_t v2,
	uint32_t v3
);

void VulkanRender::createSwapChain()
{
	SwapChainSupportDetails support = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
	VkPresentModeKHR presentMode = choosePresentMode(support.presentModes);
	VkExtent2D extent = chooseSwapExtent(support.caps);

	uint32_t imageCount = support.caps.minImageCount + 1;

	if (support.caps.maxImageCount > 0 && imageCount > support.caps.maxImageCount)
	{
		imageCount = support.caps.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<uint32_t> fams = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
		indices.transferFamily.value()
	};

	if (!_isTriBool(
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
		indices.transferFamily.value()
	))
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = fams.size();
		createInfo.pQueueFamilyIndices = fams.data();
		std::cout << "Concurrent Mode" << std::endl;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		std::cout << "Exclusive Mode" << std::endl;
	}

	createInfo.preTransform = support.caps.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult r = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Swapchain faied to be createed...");
	}

	uint32_t count = 0;
	vkGetSwapchainImagesKHR(device, swapChain, &count, nullptr);
	swapChainImages.resize(count);
	vkGetSwapchainImagesKHR(device, swapChain, &count, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

static bool _isTriBool(
	uint32_t v1,
	uint32_t v2,
	uint32_t v3
)
{
	return	v1 == v2 &&
		v1 == v3 &&
		v2 == v3;
}

SwapChainSupportDetails VulkanRender::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	uint32_t count = 0;

	// Caps
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.caps);

	// Formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
	if (count > 0)
	{
		details.formats.resize(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, details.formats.data());
	}

	count = 0;

	// PresentModes
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
	if (count > 0)
	{
		details.presentModes.resize(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanRender::chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR>& formats)
{
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& f : formats)
	{
		if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return f;
		}
	}

	return formats[0];
}

VkPresentModeKHR VulkanRender::choosePresentMode(const std::vector<VkPresentModeKHR>& modes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& m : modes)
	{
		if (m == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return m;
		}
		else if (m == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			return m;
		}
	}

	return bestMode;
}

VkExtent2D VulkanRender::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps)
{
	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return caps.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { app_getWidth(), app_getHeight() };

		actualExtent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanRender::createSwapChainImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult r = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);

		if (r != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view");
		}
	}
}

void VulkanRender::createCommandPools()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<uint32_t> is = {
		indices.graphicsFamily.value(),
		indices.transferFamily.value()
	};

	std::vector<VkCommandPoolCreateInfo> infos;

	for (uint32_t i = 0; i < is.size(); i++)
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = is[i];
		info.flags = 0;
		infos.push_back(info);
	}

	// Create Graphics Command Pool
	VkResult r = vkCreateCommandPool(device, &infos[0], nullptr, &graphicsCommandPool);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("Failed to create Graphics Command Pool");
	}

	// Create Transfer Command Pool
	r = vkCreateCommandPool(device, &infos[1], nullptr, &transferCommandPool);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("Failed to create Transfer Command Pool");
	}
}

void VulkanRender::createRenderPass()
{
	// Clear Pass
	createRenderPass(
		&clearRenderPass,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		"Clear pass wasn't created..."
	);
	// Draw Pass
	createRenderPass(
		&drawRenderPass,
		VK_ATTACHMENT_LOAD_OP_LOAD,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		"Draw pass wasn't created..."
	);

}

void VulkanRender::createRenderPass(
	VkRenderPass* pass,
	const VkAttachmentLoadOp& load,
	const VkAttachmentStoreOp& store,
	const VkImageLayout& initialLayout,
	const VkImageLayout& finalLayout,
	std::string errorMsg)
{
	// Attachment Description
	VkAttachmentDescription attachment = {};
	attachment.format = swapChainImageFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;

	attachment.loadOp = load;
	attachment.storeOp = store;

	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachment.initialLayout = initialLayout;
	attachment.finalLayout = finalLayout;

	// Attachment Reference
	VkAttachmentReference attachmentRef = {};
	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Subpass Description
	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &attachmentRef;

	// Subpass Dependency
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	dependency.srcAccessMask = 0;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Create Info
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &attachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	VkResult r = vkCreateRenderPass(device, &createInfo, nullptr, pass);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error(errorMsg);
	}
}

void VulkanRender::createFrameBuffer()
{
	// Clear Framebuffer
	createFrameBuffer(
		clearFrameBuffer,
		&clearRenderPass
	);
	
	// Draw Framebuffer
	createFrameBuffer(
		drawFrameBuffer,
		&drawRenderPass
	);
}

void VulkanRender::createFrameBuffer(
	std::vector<VkFramebuffer>& frameBuffers,
	VkRenderPass* renderPass
)
{
	frameBuffers.resize(swapChainImageViews.size());
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = *renderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &swapChainImageViews[i];
		createInfo.width = swapChainExtent.width;
		createInfo.height = swapChainExtent.height;
		createInfo.layers = 1;

		VkResult r = vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffers[i]);

		if (r != VK_SUCCESS)
		{
			std::runtime_error("Failed to create framebuffer...");
		}
	}
}

void VulkanRender::destroyFrameBuffer(std::vector<VkFramebuffer>& frameBuffers)
{
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
	}
}

void VulkanRender::createSemaphore()
{
	VkResult r;

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	r = vkCreateSemaphore(device, &createInfo, nullptr, &imageAvailable);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Image Available semaphore...");
	}

	r = vkCreateSemaphore(device, &createInfo, nullptr, &renderFinish);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Render Finished semaphore...");
	}


}

void VulkanRender::createFence()
{
	VkResult r;
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	r = vkCreateFence(
		device,
		&createInfo,
		nullptr,
		&inFlight
	);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("inflight fence... wasn't created");
	}
}

void VulkanRender::addGraphicCommand(VkCommandBuffer cmd)
{
	this->commandBufferList.push_back(cmd);
}


// Descriptor Pool
void VulkanRender::createDescPool()
{
	VkResult r;

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	r = vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &this->descPool);

}

void VulkanRender::destroyDescPool()
{
	vkDestroyDescriptorPool(this->device, this->descPool, nullptr);
}
