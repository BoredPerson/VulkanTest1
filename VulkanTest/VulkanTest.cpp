// VulkanTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include <vector>
#include <fstream>
#include "glm/glm.hpp"


#define ASSERT_VULKAN(val) if (val != VK_SUCCESS){ __debugbreak();}		//checks function return for success 

VkResult result;
VkApplicationInfo appInfo;
VkPhysicalDevice *physicalDevices;
VkInstance instance;
VkDevice device;
GLFWwindow *window;
VkSurfaceKHR surface;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
VkImageView *imageViews;
uint32_t amountOfSwapchainImages;
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
VkRenderPass renderPass;
VkPipeline pipeline;
VkFramebuffer *framebuffers;
VkCommandPool commandPool;
VkCommandBuffer *commandBuffers;
VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingDone;
VkQueue queue;
VkImage *swapchainImages;
VkLayerProperties *layers;
VkExtensionProperties *extensions;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferDeviceMemory;

const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
uint32_t width = 600;
uint32_t height = 400;

class Vertex {
public:
	glm::vec2 pos;
	glm::vec3 col;

	Vertex(glm::vec2 pos, glm::vec3 col)
		:pos(pos), col(col)
	{}

	static VkVertexInputBindingDescription getVertexInputBindingDescription() {
		VkVertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(2);
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, pos);

		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, col);

		return vertexInputAttributeDescriptions;
	}
};

std::vector<Vertex>vertices = {
	Vertex({ 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f }),
	Vertex({ 0.5f,  0.5f },{ 0.0f, 1.0f, 0.0f }),
	Vertex({-0.5f,  0.5f },{ 0.0f, 0.0f, 1.0f })
};

void printStats(VkPhysicalDevice &device) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);
	uint32_t apiVer = properties.apiVersion;
	uint32_t driverVer = properties.driverVersion;
	std::cout << "Name:	                            " << properties.deviceName << std::endl;
	std::cout << "   Properties:                      " << std::endl;
	std::cout << "      API Version:                  " << VK_VERSION_MAJOR(apiVer) << "." << VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl;
	std::cout << "      Driver Version:               " << VK_VERSION_MAJOR(driverVer) << "." << VK_VERSION_MINOR(driverVer) << "." << VK_VERSION_PATCH(driverVer) << std::endl;
	std::cout << "      Vendor ID:                    " << properties.vendorID << std::endl;
	std::cout << "      Device ID:                    " << properties.deviceID << std::endl;
	std::cout << "      Device Type:                  " << properties.deviceType << std::endl;
	std::cout << "      discreteQueuePriorities:      " << properties.limits.discreteQueuePriorities << std::endl;
	std::cout << std::endl;

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);
	std::cout << "   Features:                        " << std::endl;
	std::cout << "      Geometry shader:              " << features.geometryShader << std::endl;
	std::cout << std::endl;

	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(device, &memProp);

	uint32_t amountOfQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, nullptr);
	VkQueueFamilyProperties *familyProperties = new VkQueueFamilyProperties[amountOfQueueFamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, familyProperties);

	for (uint32_t i = 0; i < amountOfQueueFamilies; i++) {
		std::cout << "   Queue Family #"                    << i << std::endl;
		std::cout << "      Queue Count:                  " << familyProperties[i].queueCount << std::endl;
		std::cout << "      VK_QUEUE_GRAPHICS_BIT:        " << ((familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
		std::cout << "      VK_QUEUE_COMPUTE_BIT:         " << ((familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
		std::cout << "      VK_QUEUE_TRANSFER_BIT:        " << ((familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
		std::cout << "      VK_QUEUE_SPARSE_BINDING_BIT:  " << ((familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
		std::cout << "      Timestamp Valid Bits:         " << familyProperties[i].timestampValidBits << std::endl;
		std::cout << std::endl;
	}

	VkBool32 surfaceSupport = false;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport);//TODO civ
	ASSERT_VULKAN(result);

	if (!surfaceSupport) {
		std::cerr << "Surface not supported" << std::endl;
		__debugbreak();
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);
	std::cout << "   Surface Capabilities:           " << std::endl;
	std::cout << "      minImageCount:           " << surfaceCapabilities.minImageCount << std::endl;
	std::cout << "      maxImageCount:           " << surfaceCapabilities.maxImageCount << std::endl;
	std::cout << "      currentExtent:           " << surfaceCapabilities.currentExtent.width << "/" << surfaceCapabilities.currentExtent.height << std::endl;
	std::cout << "      minImageExtent:          " << surfaceCapabilities.minImageExtent.width << "/" << surfaceCapabilities.minImageExtent.height << std::endl;
	std::cout << "      maxImageExtent:          " << surfaceCapabilities.maxImageExtent.width << "/" << surfaceCapabilities.maxImageExtent.height << std::endl;
	std::cout << "      maxImageArrayLayers:     " << surfaceCapabilities.maxImageArrayLayers << std::endl;
	std::cout << "      supportedTransforms:     " << surfaceCapabilities.supportedTransforms << std::endl;
	std::cout << "      currentTransform:        " << surfaceCapabilities.currentTransform << std::endl;
	std::cout << "      supportedCompositeAlpha: " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
	std::cout << "      supportedUsageFlags:     " << surfaceCapabilities.supportedUsageFlags << std::endl;
	std::cout << std::endl;

	uint32_t amountOfFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, nullptr);
	VkSurfaceFormatKHR *formats = new VkSurfaceFormatKHR[amountOfFormats];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, formats);

	std::cout << "      Amount of Formats: " << amountOfFormats << std::endl;
	for (uint32_t i = 0; i < amountOfFormats; i++) {
		std::cout << "         Format:  " << formats[i].format << std::endl;
	}
	std::cout << std::endl;

	uint32_t amountOfPresentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentModes, nullptr);
	VkPresentModeKHR *presentModes = new VkPresentModeKHR[amountOfPresentModes];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentModes, presentModes);
	std::cout << "      Amount of Presentation Modes: " << amountOfPresentModes << std::endl;
	for (uint32_t i = 0; i < amountOfPresentModes; i++) {
		std::cout << "         Presentation Mode: " << presentModes[i] << std::endl;
	}
	std::cout << std::endl;
}

std::vector<char> readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file) {
		size_t size = static_cast<size_t>(file.tellg());
		std::vector<char> fileBuffer(size);
		file.seekg(0);
		file.read(fileBuffer.data(), size);
		file.close();
		return fileBuffer;
	}
	else {
		std::runtime_error("Failed to read file!");
	}
}

void createShaderModule(const std::vector<char> &shaderCode, VkShaderModule *shaderModule) {
	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = shaderCode.size();
	shaderCreateInfo.pCode = (uint32_t*)shaderCode.data();

	result = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule);
	ASSERT_VULKAN(result);
}

void createAppInfo() {
	//create application info
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Vulkan test";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Vulkan test engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_1;
}

void printLayers() {
	//print layers and extensions
	uint32_t amountOfLayers = 0;
	result = vkEnumerateInstanceLayerProperties(&amountOfLayers, nullptr);
	ASSERT_VULKAN(result);
	layers = new VkLayerProperties[amountOfLayers];
	result = vkEnumerateInstanceLayerProperties(&amountOfLayers, layers);
	ASSERT_VULKAN(result);

	std::cout << "Layers: " << std::endl;
	std::cout << "  Amount of Instance Layers:  " << amountOfLayers << std::endl;
	std::cout << std::endl;
	for (uint32_t i = 0; i < amountOfLayers; i++) {
		std::cout << "  Layer Name:		" << layers[i].layerName << std::endl;
		std::cout << "  	Layer Nr:       " << i << std::endl;
		std::cout << "  	Description:	" << layers[i].description << std::endl;
		std::cout << "  	Spec Version:	" << layers[i].specVersion << std::endl;
		std::cout << "  	Implementation: " << layers[i].implementationVersion << std::endl;
		std::cout << std::endl;
	}
}

void printExtensions() {
	uint32_t amountOfExtensions = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, nullptr);
	ASSERT_VULKAN(result);
	extensions = new VkExtensionProperties[amountOfExtensions];
	result = vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, extensions);

	std::cout << "Extensions: " << std::endl;
	std::cout << "  Amount of Extensions:  " << amountOfExtensions << std::endl;
	std::cout << std::endl;
	for (uint32_t i = 0; i < amountOfExtensions; i++) {
		std::cout << "  Extension Name:	" << extensions[i].extensionName << std::endl;
		std::cout << "  	Layer Nr:       " << i << std::endl;
		std::cout << "  	Spec Version:	" << extensions[i].specVersion << std::endl;
		std::cout << std::endl;
	}
}

void createInstance() {
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	//get needed extensions for glfw
	uint32_t amountOfGlfwExtensions = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);


	//create instance
	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	instanceCreateInfo.enabledExtensionCount = amountOfGlfwExtensions;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	ASSERT_VULKAN(result);
}

void createSurface() {
	//create glfw surface
	result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	ASSERT_VULKAN(result);
}

void createQueues() {
	//create queues
	vkGetDeviceQueue(device, 0, 0, &queue);
}

void createPhysicalDevice() {
	//create physical devices and print stats
	uint32_t amountOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, nullptr);
	ASSERT_VULKAN(result);
	physicalDevices = new VkPhysicalDevice[amountOfPhysicalDevices];
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices);
	ASSERT_VULKAN(result);

	for (uint32_t i = 0; i < amountOfPhysicalDevices; i++) {
		printStats(physicalDevices[i]);
	}

}

void createLogicalDevice() {
	//create queue info
	float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0;												//TODO select correct family
	deviceQueueCreateInfo.queueCount = 4;													//TODO civ
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;


	//create device
	VkPhysicalDeviceFeatures usedFeatures = {};

	const std::vector<const char*> deviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &usedFeatures;

	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);		//TODO select correct physical device
	ASSERT_VULKAN(result);
}

void createSwapchain() {

	//create swapchain 

	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;													//TODO civ
	swapchainCreateInfo.imageFormat = format;								//TODO civ
	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = VkExtent2D{ width, height };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;											//TODO civ
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;							//TODO civ
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = swapchain;

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	ASSERT_VULKAN(result);


	//create swapchain images
	vkGetSwapchainImagesKHR(device, swapchain, &amountOfSwapchainImages, nullptr);
	swapchainImages = new VkImage[amountOfSwapchainImages];
	result = vkGetSwapchainImagesKHR(device, swapchain, &amountOfSwapchainImages, swapchainImages);
	ASSERT_VULKAN(result);

}

void createImageViews() {
	//create image views
	imageViews = new VkImageView[amountOfSwapchainImages];

	VkImageViewCreateInfo imageviewCreateInfo;
	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		imageviewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageviewCreateInfo.pNext = nullptr;
		imageviewCreateInfo.flags = 0;
		imageviewCreateInfo.image = swapchainImages[i];
		imageviewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageviewCreateInfo.format = format;
		imageviewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageviewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageviewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageviewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageviewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageviewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageviewCreateInfo.subresourceRange.levelCount = 1;
		imageviewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageviewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &imageviewCreateInfo, nullptr, &imageViews[i]);
		ASSERT_VULKAN(result);
	}
}

void createPipeline() {
	//create shader modules
	auto shaderCodeVert = readFile("shader.vert.spv");
	auto shaderCodeFrag = readFile("shader.frag.spv");

	createShaderModule(shaderCodeVert, &shaderModuleVert);
	createShaderModule(shaderCodeFrag, &shaderModuleFrag);

	//create shader pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo;
	vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.pNext = nullptr;
	vertShaderStageCreateInfo.flags = 0;
	vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateInfo.module = shaderModuleVert;
	vertShaderStageCreateInfo.pName = "main";
	vertShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo;
	fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.pNext = nullptr;
	fragShaderStageCreateInfo.flags = 0;
	fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.module = shaderModuleFrag;
	fragShaderStageCreateInfo.pName = "main";
	fragShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo , fragShaderStageCreateInfo };

	auto vertexBindingDescriptions = Vertex::getVertexInputBindingDescription();
	auto vertexAttributeDescriptions = Vertex::getVertexInputAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescriptions;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { width, height };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.pNext = nullptr;
	rasterizationStateCreateInfo.flags = 0;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasConstantFactor = 0;
	rasterizationStateCreateInfo.depthBiasClamp = 0;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
	rasterizationStateCreateInfo.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.pNext = nullptr;
	multisampleStateCreateInfo.flags = 0;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.minSampleShading = 1;
	multisampleStateCreateInfo.pSampleMask = nullptr;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.pNext = nullptr;
	colorBlendStateCreateInfo.flags = 0;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.pResolveAttachments = 0;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	ASSERT_VULKAN(result);

	VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_SCISSOR , VK_DYNAMIC_STATE_VIEWPORT };

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	ASSERT_VULKAN(result);
}

void createFrameBuffers() {
	//create framebuffers
	framebuffers = new VkFramebuffer[amountOfSwapchainImages];
	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &(imageViews[i]);
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &(framebuffers[i]));
		ASSERT_VULKAN(result);
	}
}

uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags flags) {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & flags)) {
			return i;
		}
	}

	__debugbreak();
	std::runtime_error("No correct memory type found");
}

void createVertexBuffer() {
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = sizeof(Vertex) * vertices.size();
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &vertexBuffer);
	ASSERT_VULKAN(result);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &vertexBufferDeviceMemory);
	ASSERT_VULKAN(result);

	vkBindBufferMemory(device, vertexBuffer, vertexBufferDeviceMemory, 0);

	void *rawData;
	vkMapMemory(device, vertexBufferDeviceMemory, 0, bufferCreateInfo.size, 0, &rawData);
	memcpy(rawData, vertices.data(), bufferCreateInfo.size);
	vkUnmapMemory(device, vertexBufferDeviceMemory);
}

void createCommandBuffers() {
	//create command buffers
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0;		//TODO civ

	result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
	ASSERT_VULKAN(result);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = 0;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = amountOfSwapchainImages;

	commandBuffers = new VkCommandBuffer[amountOfSwapchainImages];
	result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
	ASSERT_VULKAN(result);

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
		ASSERT_VULKAN(result);

		VkRenderPassBeginInfo renderPassBeginnInfo;
		renderPassBeginnInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginnInfo.pNext = nullptr;
		renderPassBeginnInfo.renderPass = renderPass;
		renderPassBeginnInfo.framebuffer = framebuffers[i];
		renderPassBeginnInfo.renderArea.offset = { 0, 0 };
		renderPassBeginnInfo.renderArea.extent = { width, height };
		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginnInfo.clearValueCount = 1;
		renderPassBeginnInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginnInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		
		VkRect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = width;
		scissor.extent.height = height;

		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);

		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		ASSERT_VULKAN(result);
	}
}

void createSemaphores() {
	//create semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable);
	ASSERT_VULKAN(result);
	result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreRenderingDone);
	ASSERT_VULKAN(result);
}

void recreateSwapchain() {
	vkDeviceWaitIdle(device);

	VkSwapchainKHR oldSwapchain = swapchain;

	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews;

	createSwapchain();
	createImageViews();
	createFrameBuffers();
	createCommandBuffers();

	vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
}

void onWindowResized(GLFWwindow *window, int w, int h) {
	if (w <= 1 && h <= 1) { return; }

	width = w;
	height = h;
	recreateSwapchain();
}

void startGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, "Vulkan Test", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onWindowResized);
}

void startVulkan() {
	createAppInfo();
	printLayers();
	printExtensions();
	createInstance();
	createSurface();
	createPhysicalDevice();
	createLogicalDevice();
	createQueues();
	createSwapchain();
	createImageViews();
	createPipeline();
	createFrameBuffers();
	createVertexBuffer();
	createCommandBuffers();
	createSemaphores();

	//delete 
	delete[] swapchainImages;
	delete[] layers;
	delete[] extensions;
	delete[] physicalDevices;
}

void drawFrame() {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
	VkPipelineStageFlags stageMask[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.pWaitDstStageMask = stageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(commandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRenderingDone;

	result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	ASSERT_VULKAN(result);

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphoreRenderingDone;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(queue, &presentInfo);
	ASSERT_VULKAN(result);
}

void loop() {
	int i = 1;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			drawFrame();
			//std::cout << i << std::endl;
			i++;
		}
	}
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	vkFreeMemory(device, vertexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);

	vkFreeCommandBuffers(device, commandPool, amountOfSwapchainImages, commandBuffers);
	delete[] commandBuffers;

	vkDestroyCommandPool(device, commandPool, nullptr);
	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}

	delete[] framebuffers;

	vkDestroyPipeline(device, pipeline, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);
	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	delete[] imageViews;

	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main()
{
	startGlfw();
	startVulkan();
	loop();
	shutdownVulkan();
	shutdownGlfw();

	return 0;
}

