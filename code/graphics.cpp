#include "sys.h"


/*
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete()
	{
		return this->graphicsFamily.has_value() &&
			   this->presentFamily.has_value() &&
			   this->transferFamily.has_value();
	}

	void printFamilies()
	{
		std::cout << "Graphics Family " << graphicsFamily.value() << std::endl;
		std::cout << "Present Family " << presentFamily.value() << std::endl;
		std::cout << "Transfer Family " << transferFamily.value() << std::endl;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR caps;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
*/

/*
static VulkanContext data;

static void _createInstance();
static void _createDebugReport();
static void _destroyDebugReport();
static VkBool32 VKAPI_CALL _debugCB(
	VkDebugReportFlagsEXT			flags,
	VkDebugReportObjectTypeEXT		objType,
	uint64_t						obj,
	size_t							location,
	int32_t							msgCode,
	const char*						layerPrefix,
	const char*						msg,
	void*							useData
);

static void _createSurface();

static void _queryPhysicalDevice();
static bool _isDeviceSuitable(VkPhysicalDevice device);
static QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device);

static void _createDevice();

static void _createSwapChain();
static SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device);
static VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR>& formats);
static VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
static VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps);

static void _createSwapChainImageViews();

static void _createCommandPools();

static void _createRenderPass();
static void _createRenderPass(
	VkRenderPass* pass, 
	const VkAttachmentLoadOp& load, 
	const VkAttachmentStoreOp& save,
	const VkImageLayout& initialLayout,
	const VkImageLayout& finalLayout,
	std::string errorMsg);

static void createFrameBuffer();
static void createFrameBuffer(
	std::vector<VkFramebuffer>& frameBuffers,
	VkRenderPass* renderPass
);
static void destroyFrameBuffer(std::vector<VkFramebuffer>& frameBuffers);

static void createSemaphore();

static void createFence();

VulkanContext* graphic_init(bool useLayer)
{
	data.useLayer = useLayer;
	_createInstance();
	if (data.useLayer)
	{
		_createDebugReport();
	}
	_createSurface();
	_queryPhysicalDevice();
	_createDevice();
	_createSwapChain();
	_createSwapChainImageViews();
	_createCommandPools();
	_createRenderPass();
	createFrameBuffer();
	createSemaphore();
	createFence();

	//cmd_init(&data);

	return &data;
}

void graphic_release(VulkanContext** context)
{
	*context = nullptr;

	//cmd_release();

	vkDestroyFence(data.device, data.inFlight, nullptr);

	vkDestroySemaphore(data.device, data.renderFinish, nullptr);
	vkDestroySemaphore(data.device, data.imageAvailable, nullptr);

	destroyFrameBuffer(data.drawFrameBuffer);
	destroyFrameBuffer(data.clearFrameBuffer);
	
	vkDestroyRenderPass(data.device, data.drawRenderPass, nullptr);
	vkDestroyRenderPass(data.device, data.clearRenderPass, nullptr);

	vkDestroyCommandPool(data.device, data.transferCommandPool, nullptr);
	vkDestroyCommandPool(data.device, data.graphicsCommandPool, nullptr);

	for (uint32_t i = 0; i < data.swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(data.device, data.swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(data.device, data.swapChain, nullptr);

	vkDestroyDevice(data.device, nullptr);

	vkDestroySurfaceKHR(data.instance, data.surface, nullptr);

	if (data.useLayer)
	{
		_destroyDebugReport();
	}

	vkDestroyInstance(data.instance, nullptr);
}

void graphic_startFrame()
{
	vkAcquireNextImageKHR(
		data.device,
		data.swapChain,
		std::numeric_limits<uint64_t>::max(),
		data.imageAvailable,
		VK_NULL_HANDLE,
		&data.swapChainIndex
	);

	//graphics_addCommand(cmd_clear(glm::vec3(0.0f)));

}

void graphics_addCommand(VkCommandBuffer commandBuffer)
{
	data.commandBufferList.push_back(commandBuffer);
}

void graphic_endFrame()
{
	VkResult r;

	// Wait for fence
	vkWaitForFences(data.device, 1, &data.inFlight, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(data.device, 1, &data.inFlight);

	// Submit
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &data.imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = data.commandBufferList.size();
	submitInfo.pCommandBuffers = data.commandBufferList.data();

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &data.renderFinish;

	r = vkQueueSubmit(data.graphicsQueue, 1, &submitInfo, data.inFlight);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit to graphics queue...");
	}

	// Present
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &data.renderFinish;

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &data.swapChain;
	presentInfo.pImageIndices = &data.swapChainIndex;

	r = vkQueuePresentKHR(data.presentQueue, &presentInfo);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit present queue.");
	}

	vkQueueWaitIdle(data.presentQueue);

	vkFreeCommandBuffers(
		data.device,
		data.graphicsCommandPool,
		data.commandBufferList.size(),
		data.commandBufferList.data()
	);

	data.commandBufferList.clear();
}

static void _createInstance()
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


	if(data.useLayer)
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

	
	VkResult r = vkCreateInstance(&createInfo, nullptr, &data.instance);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Didn't create instance...");
	}
}

static void _createDebugReport()
{
	data.out.open("log.txt");
	data.out << "Starting Debuging..." << std::endl;
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	createInfo.pfnCallback = _debugCB;
	createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;

	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(data.instance, "vkCreateDebugReportCallbackEXT");

	if (func != nullptr)
	{
		VkResult r = func(data.instance, &createInfo, nullptr, &data.debugReportCB);

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

static void _destroyDebugReport()
{
	data.out << "Ending Debuging..." << std::endl;
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(data.instance, "vkDestroyDebugReportCallbackEXT");
	func(data.instance, data.debugReportCB, nullptr);
	data.out.close();
}

static VkBool32 VKAPI_CALL _debugCB(
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

	std::stringstream ss;

	if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) == VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		ss << "Waring) ";
	}
	else if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		ss << "Error) ";
	}
	data.out << ss.str() << layerPrefix << ": " << msg << std::endl;

	return VK_FALSE;
}

static void _createSurface()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(app_getWindow(), &info);

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = info.info.win.hinstance;
	createInfo.hwnd = info.info.win.window;

	auto func = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(data.instance, "vkCreateWin32SurfaceKHR");

	if (func != nullptr)
	{
		VkResult r = func(data.instance, &createInfo, nullptr, &data.surface);

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

static void _queryPhysicalDevice()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(data.instance, &count, nullptr);

	if (count == 0)
	{
		throw std::runtime_error("Failed to find a GPU capable with Vulkan api...");
	}

	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(data.instance, &count, devices.data());

	for (const auto& device : devices)
	{
		if (_isDeviceSuitable(device))
		{
			data.physicalDevice = device;
			break;
		}
	}

	if (data.physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU for Vulkan API!");
	}
}

static bool _isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = _findQueueFamilies(device);
	return indices.isComplete();
}

static QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device)
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, data.surface, &presentSupport);

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

static void _createDevice()
{
	QueueFamilyIndices indices = _findQueueFamilies(data.physicalDevice);

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

	if (data.useLayer)
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

	VkResult r = vkCreateDevice(data.physicalDevice, &createInfo, nullptr, &data.device);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(data.device, indices.graphicsFamily.value(), 0, &data.graphicsQueue);
	vkGetDeviceQueue(data.device, indices.presentFamily.value(), 0, &data.presentQueue);
	vkGetDeviceQueue(data.device, indices.transferFamily.value(), 0, &data.transferQueue);
}

static bool _isTriBool(
	uint32_t v1,
	uint32_t v2,
	uint32_t v3
);

static void _createSwapChain()
{
	SwapChainSupportDetails support = _querySwapChainSupport(data.physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(support.formats);
	VkPresentModeKHR presentMode = _choosePresentMode(support.presentModes);
	VkExtent2D extent = _chooseSwapExtent(support.caps);

	uint32_t imageCount = support.caps.minImageCount + 1;

	if (support.caps.maxImageCount > 0 && imageCount > support.caps.maxImageCount)
	{
		imageCount = support.caps.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = data.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = _findQueueFamilies(data.physicalDevice);

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

	VkResult r = vkCreateSwapchainKHR(data.device, &createInfo, nullptr, &data.swapChain);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("Swapchain faied to be createed...");
	}

	uint32_t count = 0;
	vkGetSwapchainImagesKHR(data.device, data.swapChain, &count, nullptr);
	data.swapChainImages.resize(count);
	vkGetSwapchainImagesKHR(data.device, data.swapChain, &count, data.swapChainImages.data());

	data.swapChainImageFormat = surfaceFormat.format;
	data.swapChainExtent = extent;


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
static SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	uint32_t count = 0;

	// Caps
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, data.surface, &details.caps);

	// Formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, data.surface, &count, nullptr);
	if (count > 0)
	{
		details.formats.resize(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, data.surface, &count, details.formats.data());
	}

	count = 0;

	// PresentModes
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, data.surface, &count, nullptr);
	if (count > 0)
	{
		details.presentModes.resize(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, data.surface, &count, details.presentModes.data());
	}

	return details;
}

static VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR>& formats)
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

static VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& modes)
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

static VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps)
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

static void _createSwapChainImageViews()
{
	data.swapChainImageViews.resize(data.swapChainImages.size());

	for (uint32_t i = 0; i < data.swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = data.swapChainImages[i];
		
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = data.swapChainImageFormat;

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

		VkResult r = vkCreateImageView(data.device, &createInfo, nullptr, &data.swapChainImageViews[i]);

		if (r != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view");
		}
	}
}

static void _createCommandPools()
{
	QueueFamilyIndices indices = _findQueueFamilies(data.physicalDevice);

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
	VkResult r = vkCreateCommandPool(data.device, &infos[0], nullptr, &data.graphicsCommandPool);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("Failed to create Graphics Command Pool");
	}

	// Create Transfer Command Pool
	r = vkCreateCommandPool(data.device, &infos[1], nullptr, &data.transferCommandPool);
	if (r != VK_SUCCESS)
	{
		std::runtime_error("Failed to create Transfer Command Pool");
	}
}

static void _createRenderPass()
{
	// Clear Pass
	_createRenderPass(
		&data.clearRenderPass,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		"Clear pass wasn't created..."
	);
	// Draw Pass
	_createRenderPass(
		&data.drawRenderPass,
		VK_ATTACHMENT_LOAD_OP_LOAD,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		"Draw pass wasn't created..."
	);


}

static void _createRenderPass(
	VkRenderPass* pass,
	const VkAttachmentLoadOp& load,
	const VkAttachmentStoreOp& store,
	const VkImageLayout& initialLayout,
	const VkImageLayout& finalLayout,
	std::string errorMsg)
{
	// Attachment Description
	VkAttachmentDescription attachment = {};
	attachment.format = data.swapChainImageFormat;
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

	VkResult r = vkCreateRenderPass(data.device, &createInfo, nullptr, pass);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error(errorMsg);
	}
}

static void createFrameBuffer()
{
	// Clear Framebuffer
	createFrameBuffer(
		data.clearFrameBuffer,
		&data.clearRenderPass
	);
	// Draw Framebuffer
	createFrameBuffer(
		data.drawFrameBuffer,
		&data.drawRenderPass
	);
}

static void createFrameBuffer(
	std::vector<VkFramebuffer>& frameBuffers,
	VkRenderPass* renderPass
)
{
	frameBuffers.resize(data.swapChainImageViews.size());
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = *renderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &data.swapChainImageViews[i];
		createInfo.width = data.swapChainExtent.width;
		createInfo.height = data.swapChainExtent.height;
		createInfo.layers = 1;

		VkResult r = vkCreateFramebuffer(data.device, &createInfo, nullptr, &frameBuffers[i]);

		if (r != VK_SUCCESS)
		{
			std::runtime_error("Failed to create framebuffer...");
		}
	}
}

static void destroyFrameBuffer(std::vector<VkFramebuffer>& frameBuffers)
{
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(data.device, frameBuffers[i], nullptr);
	}
}

static void createSemaphore()
{
	VkResult r;

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	r = vkCreateSemaphore(data.device, &createInfo, nullptr, &data.imageAvailable);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Image Available semaphore...");
	}

	r = vkCreateSemaphore(data.device, &createInfo, nullptr, &data.renderFinish);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Render Finished semaphore...");
	}


}

static void createFence()
{
	VkResult r;
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	r = vkCreateFence(
		data.device,
		&createInfo,
		nullptr,
		&data.inFlight
	);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("inflight fence... wasn't created");
	}
}

*/