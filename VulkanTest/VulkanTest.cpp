// VulkanTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "vulkan\vulkan.hpp"
#include <vector>

#define ASSERT_VULKAN(val) if (val != VK_SUCCESS){ __debugbreak();}

VkInstance instance;
VkDevice device;

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
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, NULL);

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

int main()
{
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Vulkan test";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Vulkan test engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkResult result;

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

	uint32_t amountOfExtensions = 0;
	result = vkEnumerateInstanceExtensionProperties(NULL, &amountOfExtensions, NULL);
	ASSERT_VULKAN(result)
	VkExtensionProperties *extensions = new VkExtensionProperties[amountOfExtensions];
	result = vkEnumerateInstanceExtensionProperties(NULL, &amountOfExtensions, extensions);

	std::cout << "Extensions: " << std::endl;
	std::cout << "  Amount of Extensions:  " << amountOfExtensions << std::endl;
	std::cout << std::endl;
	for (uint32_t i = 0; i < amountOfExtensions; i++) {
		std::cout << "  Extension Name:	" << extensions[i].extensionName << std::endl;
		std::cout << "  	Layer Nr:       " << i << std::endl;
		std::cout << "  	Spec Version:	" << extensions[i].specVersion << std::endl;
		std::cout << std::endl;
	}

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = validationLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	instanceCreateInfo.enabledExtensionCount = 0;
	instanceCreateInfo.ppEnabledExtensionNames = NULL;

	result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
	ASSERT_VULKAN(result)

	uint32_t amountOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, NULL);
	ASSERT_VULKAN(result)
	VkPhysicalDevice *physicalDevices = new VkPhysicalDevice[amountOfPhysicalDevices];
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices);
	ASSERT_VULKAN(result)

	for (uint32_t i = 0; i < amountOfPhysicalDevices; i++) {
		printStats(physicalDevices[i]);
	}

	float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = NULL;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0;		//TODO select correct family
	deviceQueueCreateInfo.queueCount = 4;	//TODO check queue amount
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkPhysicalDeviceFeatures usedFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = NULL;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = NULL;
	deviceCreateInfo.pEnabledFeatures = &usedFeatures;

	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, NULL, &device);		//TODO select correct physical device
	ASSERT_VULKAN(result);





	vkDeviceWaitIdle(device);

	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);

	delete[] layers;
	delete[] extensions;
	delete[] physicalDevices;

	std::cin.get();
    return 0;
}

