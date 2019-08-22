#pragma once
#include "../surface/WindowManager.hpp"
#include "util/IO.hpp"
#include <optional>
#include <set>

struct QueueFamilies
{
	std::optional<uint32> graphics;
	std::optional<uint32> present;

	bool complete()
	{
		return graphics.has_value() && present.has_value();
	}
};

struct PhysicalDevice
{
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	QueueFamilies queueFamilies;
};

struct Swapchain
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> modes;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR mode;
	VkExtent2D extent;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	std::vector<VkFramebuffer> framebuffers;
};

struct Pipeline
{
	VkPipelineLayout layout;
	VkRenderPass pass;
	VkPipeline pipeline;
	VkCommandPool pool;
	std::vector<VkCommandBuffer> buffers;
};

class VkGfx : public Module
{
public:
	bool nominal = true;
	std::vector<const char*> validationLayers;
	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	void(*destroyDebugCallback)(VkInstance_T*, VkDebugUtilsMessengerEXT_T*, const struct VkAllocationCallbacks*);
	std::vector<PhysicalDevice> physicalDevices;
	PhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceKHR surface;
	std::vector<VkQueue> deviceQueues;
	Swapchain swapchain;
	Pipeline pipeline;
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	uint32 framesInFlight = 2;
	uint32 currentFrame = 0;
	std::vector<VkFence> fencesInFlight;

	void onStart() override
	{
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");

		if (nominal) createInstance();
		if (nominal) initDebug();
		if (nominal) selectPhysicalDevice();
		if (nominal) initSurface();
		if (nominal) initDevice();
		if (nominal) initSwapchain();
		if (nominal) initPipeline();
		if (nominal) initFramebuffers();
		if (nominal) initCommandPool();
		if (nominal) initSemaphores();
	};

	void recreateSwapchain()
	{
		vkDeviceWaitIdle(device);
		cleanupSwapchain();
		if (nominal) initSwapchain();
		if (nominal) initPipeline();
		if (nominal) initFramebuffers();
		if (nominal) initCommandPool();
	}

	std::vector<const char*> getRequiredExtensions()
	{
		uint32 glfwExtensionsCount;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
		if (!validationLayers.empty())
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	bool validationLayersSupported()
	{
		uint32 count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> available(count);
		vkEnumerateInstanceLayerProperties(&count, available.data());
		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : available)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}
		return true;
	};

	void createInstance()
	{
		VkApplicationInfo ai = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		ai.pApplicationName = getParent<Module>()->getParent<Module>()->getParent<Modular>()->getModule("game")->friendlyName.c_str();
		ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		ai.pEngineName = "Viper";
		ai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		ai.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo create = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		create.pApplicationInfo = &ai;

		auto extensions = getRequiredExtensions();
		create.enabledExtensionCount = (uint32)extensions.size();
		create.ppEnabledExtensionNames = extensions.data();

		if (validationLayersSupported())
		{
			create.enabledLayerCount = (uint32)validationLayers.size();
			create.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			create.enabledLayerCount = 0;
			validationLayers.clear();
			info("VK validation layers will not be enabled");
		}

		if (vkCreateInstance(&create, nullptr, &instance) != VK_SUCCESS)
		{
			crit("VK instance could not be created");
			nominal = false;
			return;
		}
	}

	void initDebug()
	{
		if (!validationLayers.empty())
		{
			VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			info.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32
			{
				if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				{
					warn("VK validation: %s", pCallbackData->pMessage);
				}
				return VK_FALSE;
			};

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func == nullptr)
			{
				warn("vkCreateDebugUtilsMessengerEXT is not present");
				return;
			}
			func(instance, &info, nullptr, &debugMessenger);
			destroyDebugCallback = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		}
	};

	void selectPhysicalDevice()
	{
		uint32 deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			crit("No Vulkan devices detected");
			return;
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::multimap<uint32, PhysicalDevice> candidates;
		for (auto& dev : devices)
		{
			uint32 score = 0;
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(dev, &deviceProperties);
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data());
			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
			for (auto ext : availableExtensions)
			{
				requiredExtensions.erase(ext.extensionName);
			}
			if (!requiredExtensions.empty()) continue;

			PhysicalDevice pDevice = { dev, deviceProperties, deviceFeatures };

			physicalDevices.push_back(pDevice);
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100;
			candidates.insert(std::make_pair(score, pDevice));
		}

		if (candidates.empty())
		{
			warn("No available device supports required extensions");
			nominal = false;
			return;
		}

		physicalDevice = candidates.rbegin()->second;
		info("Using physical device: %s", physicalDevice.properties.deviceName);
	};

	void initSurface()
	{
		if (glfwCreateWindowSurface(instance, getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm")->window, nullptr, &surface) != VK_SUCCESS)
		{
			crit("Failed to create window surface");
			nominal = false;
			return;
		}

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.device, &queueFamilyCount, queueFamilies.data());
		int i = 0;
		for (const auto& family : queueFamilies)
		{
			if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				physicalDevice.queueFamilies.graphics = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.device, i, surface, &presentSupport);
			if (presentSupport)
			{
				physicalDevice.queueFamilies.present = i;
			}

			if (physicalDevice.queueFamilies.complete())
			{
				break;
			}

			i++;
		}
	};

	void initDevice()
	{
		std::set<uint32> indices = { physicalDevice.queueFamilies.graphics.value(), physicalDevice.queueFamilies.present.value() };
		std::vector<VkDeviceQueueCreateInfo> qInfos;
		for (uint32 ind : indices)
		{
			VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			info.queueFamilyIndex = ind;
			info.queueCount = 1;
			float priority = 1.0f;
			info.pQueuePriorities = &priority;
			qInfos.push_back(info);
		}

		VkPhysicalDeviceFeatures features = { };

		VkDeviceCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		info.queueCreateInfoCount = (uint32)qInfos.size();
		info.pQueueCreateInfos = qInfos.data();

		info.enabledExtensionCount = (uint32)deviceExtensions.size();
		info.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(physicalDevice.device, &info, nullptr, &device) != VK_SUCCESS)
		{
			crit("Failed to create device");
			nominal = false;
			return;
		}

		deviceQueues.resize(2);
		vkGetDeviceQueue(device, physicalDevice.queueFamilies.graphics.value(), 0, &deviceQueues[0]);
		vkGetDeviceQueue(device, physicalDevice.queueFamilies.present.value(), 0, &deviceQueues[1]);
	};

	void initSwapchain()
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.device, surface, &swapchain.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			swapchain.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.device, surface, &formatCount, swapchain.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			swapchain.modes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.device, surface, &presentModeCount, swapchain.modes.data());
		}

		swapchain.format = [](Swapchain& swap) -> VkSurfaceFormatKHR
		{
			for (auto& form : swap.formats)
			{
				if (form.format == VK_FORMAT_B8G8R8A8_UNORM && form.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
				{
					return form;
				}
			}
			return swap.formats[0];
		}(swapchain);

		swapchain.mode = [](Swapchain swap) -> VkPresentModeKHR
		{
			for (auto& pres : swap.modes)
			{
				if (pres == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return pres;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}(swapchain);

		swapchain.extent = [](Swapchain swap, std::shared_ptr<WindowManager> wm) -> VkExtent2D
		{
			VkExtent2D ext = { (uint32)wm->width, (uint32)wm->height };
			ext.width = std::max(swap.capabilities.minImageExtent.width, std::min(swap.capabilities.maxImageExtent.width, ext.width));
			ext.height = std::max(swap.capabilities.minImageExtent.height, std::min(swap.capabilities.maxImageExtent.height, ext.height));
			return ext;
		}(swapchain, getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm"));

		uint32 imageCount = swapchain.capabilities.minImageCount + 1;
		if (swapchain.capabilities.maxImageCount > 0 && imageCount > swapchain.capabilities.maxImageCount)
		{
			imageCount = swapchain.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		info.surface = surface;
		info.minImageCount = imageCount;
		info.imageFormat = swapchain.format.format;
		info.imageColorSpace = swapchain.format.colorSpace;
		info.imageExtent = swapchain.extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (physicalDevice.queueFamilies.graphics.value() != physicalDevice.queueFamilies.present.value())
		{
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			uint32 indices[] = { physicalDevice.queueFamilies.graphics.value(), physicalDevice.queueFamilies.present.value() };
			info.pQueueFamilyIndices = indices;
		}
		else
		{
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		info.preTransform = swapchain.capabilities.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = swapchain.mode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &info, nullptr, &swapchain.swapchain) != VK_SUCCESS)
		{
			crit("Swapchain could not be created");
			nominal = false;
			return;
		}

		vkGetSwapchainImagesKHR(device, swapchain.swapchain, &imageCount, nullptr);
		swapchain.images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain.swapchain, &imageCount, swapchain.images.data());
		swapchain.views.resize(imageCount);
		for (uint32 i = 0; i < swapchain.images.size(); i++)
		{
			VkImageViewCreateInfo inf = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			inf.image = swapchain.images[i];
			inf.viewType = VK_IMAGE_VIEW_TYPE_2D;
			inf.format = swapchain.format.format;
			inf.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			inf.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			inf.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			inf.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			inf.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			inf.subresourceRange.baseMipLevel = 0;
			inf.subresourceRange.levelCount = 1;
			inf.subresourceRange.baseArrayLayer = 0;
			inf.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &inf, nullptr, &swapchain.views[i]) != VK_SUCCESS)
			{
				crit("Failed to create image view");
				nominal = false;
			}
		}
	};

	VkShaderModule loadAndCompileShaderModule(std::string name)
	{
		// TODO: shaders aren't the correct bytecode format?
		// SPIR-V module not valid: The following forward referenced IDs have not been defined:
		// 12[% positions] 4[% main] 34[% _] 38[% 38] 49[% 49] 23[% colors] 32[% gl_PerVertex]
		/*auto source = readFile("shaders" + seperator() + name);
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.c_str(), kind, name.c_str(), options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			crit("Shader compilation failed: %s", result.GetErrorMessage().c_str());
			return nullptr;
		}
		std::vector<uint32> code(result.cbegin(), result.cend());
		VkShaderModuleCreateInfo sinfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		sinfo.codeSize = code.size();
		sinfo.pCode = code.data();*/

		auto code = readFile("shaders" + seperator() + name);
		VkShaderModuleCreateInfo sinfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		sinfo.codeSize = code.size();
		sinfo.pCode = reinterpret_cast<const uint32*>(code.data());
		VkShaderModule shader;
		if (vkCreateShaderModule(device, &sinfo, nullptr, &shader) != VK_SUCCESS)
		{
			crit("Unable to create shader module");
			return nullptr;
		}
		return shader;
	};

	void initPipeline()
	{
		auto vert = loadAndCompileShaderModule("basic.vert.spv");
		auto frag = loadAndCompileShaderModule("basic.frag.spv");
		if (vert == 0 || frag == 0)
		{
			nominal = false;
			crit("Pipeline creation stopped due to null shader modules");
			return;
		}
		VkPipelineShaderStageCreateInfo vertStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageInfo.module = vert;
		vertStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageInfo.module = frag;
		fragStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };
		VkPipelineVertexInputStateCreateInfo vertexInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInfo.vertexBindingDescriptionCount = 0;
		vertexInfo.pVertexBindingDescriptions = nullptr;
		vertexInfo.vertexAttributeDescriptionCount = 0;
		vertexInfo.pVertexAttributeDescriptions = nullptr;
		VkPipelineInputAssemblyStateCreateInfo assembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = { };
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.extent.width;
		viewport.height = (float)swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = { };
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.extent;
		VkPipelineViewportStateCreateInfo viewportInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthClampEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.setLayoutCount = 0;

		if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipeline.layout) != VK_SUCCESS)
		{
			crit("Pipeline layout creation has failed");
			nominal = false;
			return;
		}

		VkAttachmentDescription attachment = { };
		attachment.format = swapchain.format.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference ref = { };
		ref.attachment = 0;
		ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = { };
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &ref;

		VkSubpassDependency dep = { };
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.srcAccessMask = 0;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo passInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &attachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = &dep;

		if (vkCreateRenderPass(device, &passInfo, nullptr, &pipeline.pass) != VK_SUCCESS)
		{
			crit("Renderpass creation has failed");
			nominal = false;
			return;
		}

		VkGraphicsPipelineCreateInfo info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		info.stageCount = 2;
		info.pStages = shaderStages;
		info.pVertexInputState = &vertexInfo;
		info.pInputAssemblyState = &assembly;
		info.pViewportState = &viewportInfo;
		info.pRasterizationState = &rasterizer;
		info.pMultisampleState = &multisampling;
		info.pColorBlendState = &colorBlending;
		info.layout = pipeline.layout;
		info.renderPass = pipeline.pass;
		info.subpass = 0;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline.pipeline) != VK_SUCCESS)
		{
			crit("Pipeline creation failed");
			nominal = false;
		}

		vkDestroyShaderModule(device, vert, nullptr);
		vkDestroyShaderModule(device, frag, nullptr);
	};

	void initFramebuffers()
	{
		swapchain.framebuffers.resize(swapchain.views.size());
		for (uint32 i = 0; i < swapchain.views.size(); i++)
		{
			VkImageView attachments[] = { swapchain.views[i] };
			VkFramebufferCreateInfo info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			info.renderPass = pipeline.pass;
			info.attachmentCount = 1;
			info.pAttachments = attachments;
			info.width = swapchain.extent.width;
			info.height = swapchain.extent.height;
			info.layers = 1;
			if (vkCreateFramebuffer(device, &info, nullptr, &swapchain.framebuffers[i]) != VK_SUCCESS)
			{
				crit("Framebuffer creation failed: index %d", i);
				nominal = false;
				return;
			}
		}
	};

	void initCommandPool()
	{
		VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		info.queueFamilyIndex = physicalDevice.queueFamilies.graphics.value();
		info.flags = 0;

		if (vkCreateCommandPool(device, &info, nullptr, &pipeline.pool) != VK_SUCCESS)
		{
			crit("Command pool creation failed");
			nominal = false;
			return;
		}

		pipeline.buffers.resize(swapchain.framebuffers.size());
		VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		alloc.commandPool = pipeline.pool;
		alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc.commandBufferCount = (uint32)pipeline.buffers.size();

		if (vkAllocateCommandBuffers(device, &alloc, pipeline.buffers.data()) != VK_SUCCESS)
		{
			crit("Could not allocate command buffers");
			nominal = false;
			return;
		}

		for (uint32 i = 0; i < (uint32)pipeline.buffers.size(); i++)
		{
			VkCommandBufferBeginInfo begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			if (vkBeginCommandBuffer(pipeline.buffers[i], &begin) != VK_SUCCESS)
			{
				crit("Failed to begin recording command buffer");
				nominal = false;
				return;
			}

			VkRenderPassBeginInfo pass = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			pass.renderPass = pipeline.pass;
			pass.framebuffer = swapchain.framebuffers[i];
			pass.renderArea.offset = { 0, 0 };
			pass.renderArea.extent = swapchain.extent;
			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
			pass.clearValueCount = 1;
			pass.pClearValues = &clearColor;

			vkCmdBeginRenderPass(pipeline.buffers[i], &pass, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(pipeline.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
			vkCmdDraw(pipeline.buffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(pipeline.buffers[i]);

			if (vkEndCommandBuffer(pipeline.buffers[i]) != VK_SUCCESS)
			{
				crit("Failed to end recording command buffer");
				nominal = false;
				return;
			}
		}
	};

	void initSemaphores()
	{
		imageAvailable.resize(framesInFlight);
		renderFinished.resize(framesInFlight);
		fencesInFlight.resize(framesInFlight);

		VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (uint32 i = 0; i < framesInFlight; i++)
		{
			if (vkCreateSemaphore(device, &info, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &info, nullptr, &renderFinished[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fInfo, nullptr, &fencesInFlight[i]) != VK_SUCCESS)
			{
				crit("Failed to create sync objects for frame %d", i);
				nominal = false;
			}
		}
	};

	void draw()
	{
		vkWaitForFences(device, 1, &fencesInFlight[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(device, 1, &fencesInFlight[currentFrame]);

		uint32 index = 0;
		vkAcquireNextImageKHR(device, swapchain.swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &index);

		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		VkSemaphore wait[] = { imageAvailable[currentFrame] };
		VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = wait;
		submit.pWaitDstStageMask = stages;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &pipeline.buffers[index];
		VkSemaphore signal[] = { renderFinished[currentFrame] };
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = signal;

		if (vkQueueSubmit(deviceQueues[0], 1, &submit, fencesInFlight[currentFrame]) != VK_SUCCESS)
		{
			warn("Failed to submit draw command");
			return;
		}

		VkPresentInfoKHR present = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = signal;
		VkSwapchainKHR swaps[] = { swapchain.swapchain };
		present.swapchainCount = 1;
		present.pSwapchains = swaps;
		present.pImageIndices = &index;

		vkQueuePresentKHR(deviceQueues[1], &present);
		currentFrame = (currentFrame + 1) % framesInFlight;
	};

	void cleanupSwapchain()
	{
		vkDestroyCommandPool(device, pipeline.pool, nullptr);
		for (auto buffer : swapchain.framebuffers)
		{
			vkDestroyFramebuffer(device, buffer, nullptr);
		}
		vkDestroyRenderPass(device, pipeline.pass, nullptr);
		vkDestroyPipeline(device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
		for (auto view : swapchain.views)
		{
			vkDestroyImageView(device, view, nullptr);
		}
		vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
	};

	void onShutdown() override
	{
		if (nominal)
		{
			vkDeviceWaitIdle(device);
			for (uint32 i = 0; i < framesInFlight; i++)
			{
				vkDestroySemaphore(device, imageAvailable[i], nullptr);
				vkDestroySemaphore(device, renderFinished[i], nullptr);
				vkDestroyFence(device, fencesInFlight[i], nullptr);
			}
			cleanupSwapchain();
			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyDevice(device, nullptr);
			if (!validationLayers.empty())
			{
				destroyDebugCallback(instance, debugMessenger, nullptr);
			}
			vkDestroyInstance(instance, nullptr);
		}
	};
};