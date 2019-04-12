#pragma once
#include "Pipeline.h"
#include "../V3.h"
#include "../view/ViewLayer.h"
#include "../util/FileUtils.h"
#include <assert.h>
#include <optional>
#include <algorithm>
#include <limits>
#include <fstream>

#define vkchck(call) do { VkResult res = call; assert(res == VK_SUCCESS); } while(0)

class PipelineVk : public Pipeline
{
public:
	class VkShader : public Shader
	{
	public:
		std::vector<VkShaderModule> modules;
		std::vector<VkPipelineShaderStageCreateInfo> stages;

		void setUniform(std::string name, UniformValue value) override
		{

		};
	};

	// ### LOGGING ###
	inline static VKAPI_ATTR VkBool32 VKAPI_CALL vkdb(VkDebugUtilsMessageSeverityFlagBitsEXT sev, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* cb, void* data)
	{
		if (sev >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			debugf("[vkdb]: %s", cb->pMessage);
		}
		return VK_FALSE;
	};

	inline static VkResult initDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, const VkAllocationCallbacks* alloc, VkDebugUtilsMessengerEXT* messenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, info, alloc, messenger);
		}
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	};

	inline static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	inline static VkResult resCast(bool b) { return (b == true ? VK_SUCCESS : VK_NOT_READY); };


	static std::vector<char> readFile(const std::string& filename) {
		auto s = FileUtils::getPathSeperator();
		std::ifstream file(FileUtils::getWorkingDirectory() + s + "resources" + s + "shaders" + s + filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}

		size_t size = (size_t)file.tellg();
		std::vector<char> buffer(size);
		file.seekg(0);
		file.read(buffer.data(), size);

		return buffer;
	}
	// #!# LOGGING #!#

	// ### INIT FUNCTIONS ###
	inline void createInstance()
	{
		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pEngineName = "V3";
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instInfo.pApplicationInfo = &appInfo;

		uint32 glfwExts;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExts);
		debugf("GLFW extensions: %d", glfwExts);
		std::vector<const char*> vec(glfwExtensions, glfwExtensions + glfwExts);
		std::vector<const char*> extensions;
		for (auto e : vec)
		{
			extensions.push_back(e);
		}

		uint32 extCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		std::vector<VkExtensionProperties> extProps(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProps.data());

		uint32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layerProps(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data());

#ifdef V3_DEBUG
		std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
		instInfo.enabledLayerCount = (uint32)validationLayers.size();
		instInfo.ppEnabledLayerNames = validationLayers.data();
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		instInfo.enabledExtensionCount = extensions.size();
		instInfo.ppEnabledExtensionNames = extensions.data();

		vkchck(vkCreateInstance(&instInfo, 0, &vkInstance));

		std::string exts;
		for (uint32 i = 0; i < extensions.size(); i++)
		{
			exts += std::string(extensions[i]) + (i == extensions.size() - 1 ? "" : ", ");
		}
		debugf("Instance extensions: %s", exts.c_str());
		debug("[vkinit] Instance created");
	};

	inline void initDebug()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = vkdb;
		createInfo.pUserData = nullptr;

		vkchck(initDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger));
		debug("[vkinit] Debug layers initialized");
	}

	inline void createSurface()
	{
		vkchck(glfwCreateWindowSurface(vkInstance, view->getWindow(), nullptr, &surface));
		debug("[vkinit] Surface created");
	};

	inline void selectDevice()
	{
		uint32 deviceCount;
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
		// Vk driver not installed
		vkchck(resCast(deviceCount >= 1));

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
		debugf("Number of physical devices: %d", devices.size());

		VkPhysicalDeviceProperties deviceProps;
		VkPhysicalDevice device = 0;
		for (uint32 i = 0; i < devices.size(); i++)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
			vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);
			
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
			{
				device = devices[i];
				deviceProps = deviceProperties;
				break;
			}

			// No suitable devices
			vkchck(resCast(i == devices.size() - 1 == false));
		}

		uint32 queueFamCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamCount, nullptr);
		std::vector<VkQueueFamilyProperties> props(queueFamCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamCount, props.data());
		
		for (uint32 i = 0; i < props.size(); i++)
		{
			auto p = props[i];
			if (p.queueCount > 0)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport)
				{
					presentIndex = i;
				}
				if (p.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					graphicsIndex = i;
				}
			}
		}

		vkchck(resCast(presentIndex >= 0));

		physDevice = device;

		
		debugf("[vkinit] Device selected: %s", deviceProps.deviceName);
	}

	inline std::vector<VkDeviceQueueCreateInfo> createQueues()
	{
		std::vector<VkDeviceQueueCreateInfo> vec;
		float priority = 1.0f;
		VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		info.queueFamilyIndex = presentIndex;
		info.queueCount = 1;
		info.pQueuePriorities = &priority;
		vec.push_back(info);
		return vec;

		debug("[vkinit] Queues created");
	};

	inline void createLogicalDevice()
	{

		VkPhysicalDeviceFeatures features = {};

		std::vector<const char*> extensions = { "VK_KHR_swapchain" };

		VkDeviceCreateInfo dInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		dInfo.queueCreateInfoCount = 1;
		dInfo.pEnabledFeatures = &features;
		auto queues = createQueues();
		dInfo.queueCreateInfoCount = queues.size();
		dInfo.pQueueCreateInfos = queues.data();
		dInfo.enabledExtensionCount = extensions.size();
		dInfo.ppEnabledExtensionNames = extensions.data();

		vkchck(vkCreateDevice(physDevice, &dInfo, nullptr, &logicalDevice));
		vkGetDeviceQueue(logicalDevice, presentIndex, 0, &presentQueue);

		debug("[vkinit] Logical device created");
	};

	inline void createSwapchain()
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &capabilities);

		uint32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, formats.data());
		}
		uint32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, presentModes.data());
		}

		VkSurfaceFormatKHR format;
		if (formats[0].format == VK_FORMAT_UNDEFINED)
		{
			format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		else
		{
			bool found = false;
			for (const auto& avail : formats)
			{
				if (avail.format == VK_FORMAT_B8G8R8A8_UNORM && avail.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					format = avail;
					found = true;
					break;
				}
			}
			if (!found) format = formats[0];
		}

		VkPresentModeKHR present;
		if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
		{
			present = VK_PRESENT_MODE_MAILBOX_KHR;
		}
		else
		{
			present = VK_PRESENT_MODE_FIFO_KHR;
		}

// APPARENTLY defined in windows.h
// TODO: define NOMINMAX, glfw?
// Thanks, Microsoft
#undef max
#undef min
		VkExtent2D extent;
		if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
		{
			extent = capabilities.currentExtent;
		}
		else
		{
			VkExtent2D ext = { view->viewWidth, view->viewHeight };
			ext.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, ext.width));
			ext.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, ext.height));
			extent = ext;
		}

		uint32 imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		info.surface = surface;
		info.minImageCount = imageCount;
		info.imageFormat = format.format;
		info.imageColorSpace = format.colorSpace;
		info.imageExtent = extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (graphicsIndex != presentIndex)
		{
			uint32 indices[] = { 0, presentIndex };
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = indices;
		}
		else
		{
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		info.preTransform = capabilities.currentTransform;

		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		info.presentMode = present;
		info.clipped = VK_TRUE;

		info.oldSwapchain = VK_NULL_HANDLE;

		vkchck(vkCreateSwapchainKHR(logicalDevice, &info, nullptr, &swapchain));

		swapFormat = format.format;
		swapExtent = extent;
		vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
		swapImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapImages.data());

		debug("[vkinit] Swapchain created");
	};

	inline void createImageViews()
	{
		swapViews.resize(swapImages.size());

		for (size_t i = 0; i < swapImages.size(); i++)
		{
			VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			info.image = swapImages[i];
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = swapFormat;
			info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.layerCount = 1;

			vkchck(vkCreateImageView(logicalDevice, &info, nullptr, &swapViews[i]));
		}

		debug("[vkinit] Image views created");
	}

	inline void createShader(std::string name) override
	{
		auto vertShaderCode = readFile(name + ".vert.spv");
		auto fragShaderCode = readFile(name + ".frag.spv");
		std::vector<VkShaderModule> vec;
		for (uint32 i = 0; i < 2; i++)
		{
			std::vector<char> code = (i == 0 ? vertShaderCode : fragShaderCode);
			VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			info.codeSize = code.size();
			info.pCode = reinterpret_cast<const uint32*>(code.data());

			VkShaderModule module;
			vkchck(vkCreateShaderModule(logicalDevice, &info, nullptr, &module));
			vec.push_back(module);
		}
		VkShader shader;
		shader.modules = vec;
		loadedShaders[name] = shader;

		VkPipelineShaderStageCreateInfo vertInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertInfo.module = loadedShaders[name].modules[0];
		vertInfo.pName = "main";
		loadedShaders[name].stages.push_back(vertInfo);

		VkPipelineShaderStageCreateInfo fragInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragInfo.module = loadedShaders[name].modules[1];
		fragInfo.pName = "main";
		loadedShaders[name].stages.push_back(fragInfo);

		debugf("[vkinit] Shader loaded: %s", name.c_str());
	};

	inline void createPipeline()
	{
		VkPipelineVertexInputStateCreateInfo vertInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertInfo.vertexBindingDescriptionCount = 0;
		vertInfo.pVertexBindingDescriptions = nullptr;
		vertInfo.vertexAttributeDescriptionCount = 0;
		vertInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo assembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = { };
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapExtent.width;
		viewport.height = (float)swapExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = { };
		scissor.offset = { 0, 0 };
		scissor.extent = swapExtent;

		VkPipelineViewportStateCreateInfo state = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		state.viewportCount = 1;
		state.pViewports = &viewport;
		state.scissorCount = 1;
		state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color = { };
		color.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blend = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.logicOpEnable = VK_FALSE;
		blend.logicOp = VK_LOGIC_OP_COPY;
		blend.attachmentCount = 1;
		blend.pAttachments = &color;
		blend.blendConstants[0] = 0.0f;
		blend.blendConstants[1] = 0.0f;
		blend.blendConstants[2] = 0.0f;
		blend.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		vkchck(vkCreatePipelineLayout(logicalDevice, &layoutInfo, nullptr, &pipelineLayout));


	};
	// #!# INIT FUNCTIONS #!#

	inline void onStartup() override
	{
		debug("Rendering pipeline: Vulkan");
		view = v3->getModule<ViewLayer>();

		createInstance();
#ifdef V3_DEBUG
		initDebug();
#endif
		createSurface();
		selectDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createShader("basic");

		debug("[vkinit] Init done");
	};

	inline void onTick() override
	{
		WorldState state;
		while (stateUpdates.try_dequeue(state));

		glfwSwapBuffers(view->getWindow());
	};

	inline void onShutdown() override
	{
		destroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
		vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
		vkDestroySurfaceKHR(vkInstance, surface, nullptr);
		vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
		for (auto v : swapViews)
		{
			vkDestroyImageView(logicalDevice, v, nullptr);
		}
		for (auto kv : loadedShaders)
		{
			for (auto shader : (&kv)->second.modules)
			{
				vkDestroyShaderModule(logicalDevice, shader, nullptr);
			}
		}
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
		vkDestroyDevice(logicalDevice, nullptr);
		vkDestroyInstance(vkInstance, 0);
	};
private:
	// ### VARS ###
	boost::container::flat_map<std::string, VkShader> loadedShaders;

	ViewLayer* view;
	VkInstance vkInstance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physDevice;
	VkDevice logicalDevice;
	uint32 presentIndex;
	uint32 graphicsIndex;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapImages;
	VkFormat swapFormat;
	VkExtent2D swapExtent;
	std::vector<VkImageView> swapViews;
	VkPipelineLayout pipelineLayout;
	// #!# VARS #!#
};