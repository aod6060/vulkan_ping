#pragma once

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

struct UniformVS
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;

	UniformVS()
	{
		proj = glm::mat4(1.0f);
		view = glm::mat4(1.0f);
		model = glm::mat4(1.0f);
	}
};


class VulkanRender
{
private:
	// Instance
	static VulkanRender* _inst;

	// Check to Use Layers
	bool useLayer = false;

	// Instance
	VkInstance instance;

	// Debug Callback
	VkDebugReportCallbackEXT debugReportCB;
	std::ofstream out;

	// Surface
	VkSurfaceKHR surface;

	// Physical Device
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	// Device
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;

	// Swap Chain
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	uint32_t swapChainIndex = 0;

	// Swap Chain Image Views
	std::vector<VkImageView> swapChainImageViews;

	// Command Pools
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;

	// Command Buffer List
	std::vector<VkCommandBuffer> commandBufferList;

	// Render Pass
	VkRenderPass clearRenderPass;
	VkRenderPass drawRenderPass;

	// Framebuffer
	std::vector<VkFramebuffer> clearFrameBuffer;
	std::vector<VkFramebuffer> drawFrameBuffer;

	// Semaphores
	VkSemaphore imageAvailable;
	VkSemaphore renderFinish;

	// Fence
	VkFence inFlight;

	// Descriptor Pool
	VkDescriptorPool descPool;

	// Vertex Buffer
	std::vector<glm::vec3> verticesList;
	// Vertices Staging
	VkBuffer verticesStagingBuffer;
	VkDeviceMemory verticesStagingDeviceMemory;
	// Vertices Buffer
	VkBuffer verticesBuffer;
	VkDeviceMemory verticesDeviceMemory;

	// Index Buffer
	std::vector<uint32_t> indexList;
	// Staging
	VkBuffer indexStagingBuffer;
	VkDeviceMemory indexStagingDeviceMemory;
	// Buffer
	VkBuffer indexBuffer;
	VkDeviceMemory indexDeviceMemory;

	// Uniform Buffer
	UniformVS uniformVS;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformDeviceMemory;
	VkDescriptorSetLayout uniformDescLayoutSet;
	VkDescriptorSet uniformDescSet;

	// Graphics
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	// -- Context related stuff for 
	void createInstance();
	void createDebugReport();
	void destroyDebugReport();
	static VkBool32 VKAPI_CALL debugCB(
		VkDebugReportFlagsEXT			flags,
		VkDebugReportObjectTypeEXT		objType,
		uint64_t						obj,
		size_t							location,
		int32_t							msgCode,
		const char*						layerPrefix,
		const char*						msg,
		void*							useData
	);

	void createSurface();

	void queryPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createDevice();

	void createSwapChain();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps);

	void createSwapChainImageViews();

	void createCommandPools();

	void createRenderPass();
	void createRenderPass(
		VkRenderPass* pass,
		const VkAttachmentLoadOp& load,
		const VkAttachmentStoreOp& save,
		const VkImageLayout& initialLayout,
		const VkImageLayout& finalLayout,
		std::string errorMsg);


	void createFrameBuffer();
	void createFrameBuffer(
		std::vector<VkFramebuffer>& frameBuffers,
		VkRenderPass* renderPass
	);

	void destroyFrameBuffer(std::vector<VkFramebuffer>& frameBuffers);
	void createSemaphore();
	void createFence();

	// Descriptor Pool
	void createDescPool();
	void destroyDescPool();

	// Vertices Buffer
	void createVerticesBuffer();
	void destroyVerticesBuffer();

	// Index Buffer
	void createIndexBuffer();
	void destroyIndexBuffer();

	// Uniform Buffer
	void createUniformBuffer();
	void destroyUniformBuffer();
	void updateUniformBuffer();

	// Graphics Pipeline
	void createGraphicsPipeline();
	void destroyGraphicsPipeline();
	std::vector<char> readFile(const std::string& path);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	// Add Command Buffer
	// Graphics Commands
	VkCommandBuffer allocateGraphicsCB();
	VkCommandBuffer clearGraphicsCB(const glm::vec3& color);
	VkCommandBuffer updateTransformCB();
	VkCommandBuffer drawCommand(
		VkBuffer* vertexBuffer,
		VkBuffer* indexBuffer,
		size_t count
	);

	void addGraphicCommand(VkCommandBuffer cmd);

	// Transfer Commands
	VkCommandBuffer allocateTransferCB();
	void freeTransferCB(VkCommandBuffer buffer);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// Buffers
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);
	void createBuffer(
		VkBuffer* buf,
		VkDeviceMemory* memory,
		size_t size,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		VkMemoryPropertyFlags memProps
	);
	void uploadBufferData(VkDeviceMemory* memory, void* data, size_t size);


public:
	static VulkanRender* getInstance();
	static void destroy();


	void init(bool useLayer);

	void release();

	void startFrame();

	void endFrame();

	void clear(const glm::vec3& clear);

	void setProj(const glm::mat4& proj);
	void setView(const glm::mat4& view);
	void setModel(const glm::mat4& model);
	void updateTransforms();
	void drawQuad();

	
};