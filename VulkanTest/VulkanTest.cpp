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
}

void startGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(800, 600, "Vulkan Test", nullptr, nullptr);
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
	result = vkEnumerateInstanceLayerProperties(&amountOfLayers, NULL);
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

	const std::vector<const char*> usedExtensions = {

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


	//create instance
	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = validationLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	instanceCreateInfo.enabledExtensionCount = usedExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = usedExtensions.data();

	result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
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
	deviceQueueCreateInfo.queueFamilyIndex = 0;		//TODO select correct family
	deviceQueueCreateInfo.queueCount = 4;	//TODO check queue amount
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;


	//create device
	VkPhysicalDeviceFeatures usedFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = &usedFeatures;

	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);		//TODO select correct physical device
	ASSERT_VULKAN(result);


	//create queues
	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);

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

	vkDestroyDevice(device, nullptr);
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

