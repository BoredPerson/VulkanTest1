// VulkanTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include <vector>

#define ASSERT_VULKAN(val) if (val != VK_SUCCESS){ __debugbreak();}		//checks function return for success 

VkInstance instance;
VkDevice device;
GLFWwindow *window;
VkSurfaceKHR surface;
VkSwapchainKHR swapchain;
VkImageView *imageViews;
uint32_t amountOfSwapchainImages;

const int WIDTH = 800;
const int HEIGHT = 600;

//prints stats of physical device
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



void startGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Test", nullptr, nullptr);
}



void startVulkan() {
	VkResult result;

	//create application info
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Vulkan test";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Vulkan test engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_1;


	//print layers and extensions
	uint32_t amountOfLayers = 0;
	result = vkEnumerateInstanceLayerProperties(&amountOfLayers, nullptr);
	ASSERT_VULKAN(result);
	VkLayerProperties *layers = new VkLayerProperties[amountOfLayers];
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

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	uint32_t amountOfExtensions = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, nullptr);
	ASSERT_VULKAN(result)
		VkExtensionProperties *extensions = new VkExtensionProperties[amountOfExtensions];
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
	ASSERT_VULKAN(result)


		//create glfw surface
		result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	ASSERT_VULKAN(result)


		//create physical devices and print stats
		uint32_t amountOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, nullptr);
	ASSERT_VULKAN(result)
		VkPhysicalDevice *physicalDevices = new VkPhysicalDevice[amountOfPhysicalDevices];
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices);
	ASSERT_VULKAN(result)

		for (uint32_t i = 0; i < amountOfPhysicalDevices; i++) {
			printStats(physicalDevices[i]);
		}


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


	//create queues
	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);


	//create swapchain 
	VkBool32 surfaceSupport = false;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport);//TODO civ
	ASSERT_VULKAN(result)

		if (!surfaceSupport) {
			std::cerr << "Surface not supported" << std::endl;
			__debugbreak();
		}

	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;													//TODO civ
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;								//TODO civ
	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = VkExtent2D{ WIDTH, HEIGHT };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;											//TODO civ
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;							//TODO civ
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	ASSERT_VULKAN(result)


	//create swapchain images
	vkGetSwapchainImagesKHR(device, swapchain, &amountOfSwapchainImages, nullptr);
	VkImage *swapchainImages = new VkImage[amountOfSwapchainImages];
	result = vkGetSwapchainImagesKHR(device, swapchain, &amountOfSwapchainImages, swapchainImages);
	ASSERT_VULKAN(result)


	//create image views
	imageViews = new VkImageView[amountOfSwapchainImages];

	VkImageViewCreateInfo imageviewCreateInfo;
	for (uint32_t i = 0; i < amountOfSwapchainImages; i++){
		imageviewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageviewCreateInfo.pNext = nullptr;
		imageviewCreateInfo.flags = 0;
		imageviewCreateInfo.image = swapchainImages[i];
		imageviewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageviewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
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
		ASSERT_VULKAN(result)
	}

	//delete arrays
	delete[] swapchainImages;
	delete[] layers;
	delete[] extensions;
	delete[] physicalDevices;
}

void loop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	for (uint32_t i = 0; i < amountOfSwapchainImages; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews;
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw() {
	glfwDestroyWindow(window);
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

