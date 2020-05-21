#include "sys.h"


// Graphics Pipeline
void VulkanRender::createGraphicsPipeline()
{
	VkResult r;

	// Create Shader Stage
	auto vertShaderCode = this->readFile("data/shaders/main.vert.spv");
	auto fragShaderCode = this->readFile("data/shaders/main.frag.spv");

	this->vertShaderModule = this->createShaderModule(vertShaderCode);
	this->fragShaderModule = this->createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// Fixed Function Stages

	// Vertices Bind
	VkVertexInputBindingDescription verticesBindDesc = {};
	verticesBindDesc.binding = 0;
	verticesBindDesc.stride = sizeof(glm::vec3);
	verticesBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputBindingDescription> bindDescs = {
		verticesBindDesc
	};

	// Vertices Attr
	VkVertexInputAttributeDescription verticesAttrDesc = {};
	verticesAttrDesc.binding = 0;
	verticesAttrDesc.location = 0;
	verticesAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	verticesAttrDesc.offset = 0;

	std::vector<VkVertexInputAttributeDescription> attrDescs = {
		verticesAttrDesc
	};

	// Vertex Input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = attrDescs.size();
	vertexInputInfo.pVertexAttributeDescriptions = attrDescs.data();
	vertexInputInfo.vertexBindingDescriptionCount = bindDescs.size();
	vertexInputInfo.pVertexBindingDescriptions = bindDescs.data();

	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport and Scissor

	// Viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)this->swapChainExtent.width;
	viewport.height = (float)this->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapChainExtent;

	// Viewport State
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	
	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthBiasClamp = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisamp = {};
	multisamp.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamp.sampleShadingEnable = VK_FALSE;
	multisamp.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


	// Depth and Stencil Testing
	// note: I won't be using Depth Buffer

	// Color Blend Attachment State...
	VkPipelineColorBlendAttachmentState colorBlendAttr = {};
	colorBlendAttr.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	colorBlendAttr.blendEnable = VK_FALSE;

	std::vector<VkPipelineColorBlendAttachmentState> blendAttrs = {
		colorBlendAttr
	};

	VkPipelineColorBlendStateCreateInfo colorBlend = {};
	colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlend.logicOpEnable = VK_FALSE;
	colorBlend.logicOp = VK_LOGIC_OP_COPY;
	colorBlend.attachmentCount = blendAttrs.size();
	colorBlend.pAttachments = blendAttrs.data();

	// Dynamic State

	/*
	// Pipeline Layout
	VkDescriptorSetLayoutBinding vertexLayoutBinding = {};
	vertexLayoutBinding.binding = 0;
	vertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertexLayoutBinding.descriptorCount = 1;
	vertexLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertexLayoutBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		vertexLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();
	 = vkCreateDescriptorSetLayout(this->device, &layoutInfo, nullptr, &this->descSetLayout);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("error failed to create descriptor set layout!");
	}
	*/

	std::vector< VkDescriptorSetLayout> layouts = { this->uniformDescLayoutSet };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = layouts.size();
	pipelineLayoutInfo.pSetLayouts = layouts.data();

	r = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("vulkan pipeline layout wasn't create.");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisamp;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlend;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = this->pipelineLayout;

	pipelineInfo.renderPass = this->clearRenderPass;
	pipelineInfo.subpass = 0;

	r = vkCreateGraphicsPipelines(
		this->device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&this->graphicsPipeline
	);

	if (r != VK_SUCCESS)
	{
		std::runtime_error("Error: Graphics Pipeline wasn't created...");
	}
}

void VulkanRender::destroyGraphicsPipeline()
{
	vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
	vkDestroyShaderModule(device, this->vertShaderModule, nullptr);
	vkDestroyShaderModule(device, this->fragShaderModule, nullptr);
}

std::vector<char> VulkanRender::readFile(const std::string& path)
{
	std::ifstream in(path, std::ios::ate | std::ios::binary);

	if (!in.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)in.tellg();
	std::vector<char> buffer(fileSize);

	in.seekg(0);
	in.read(buffer.data(), fileSize);

	in.close();

	return buffer;
}

VkShaderModule VulkanRender::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule temp;

	VkResult r = vkCreateShaderModule(device, &createInfo, nullptr, &temp);

	if (r != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return temp;
}