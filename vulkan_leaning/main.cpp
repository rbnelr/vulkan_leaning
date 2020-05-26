#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "kissmath.hpp"
#include "assert.h"
#include <vector>
#include "util/file_io.hpp"

const int2 window_size = int2(1280, 720);

GLFWwindow* glfw_window;

// https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

const bool vk_enable_validation_layers = true;
const std::vector<char const*> vk_validation_layers = {
	"VK_LAYER_KHRONOS_validation",
};

std::vector<char const*> vk_layers;

const std::vector<char const*> vk_device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

VkInstance						vk_instance;
VkSurfaceKHR					vk_surface;
VkPhysicalDevice				vk_physical_device;
VkDevice						vk_device;
VkQueue							vk_graphics_queue;
VkQueue							vk_present_queue;
VkSwapchainKHR					vk_swap_chain;
std::vector<VkImage>			vk_swap_chain_images;
VkFormat						vk_swap_chain_image_format;
VkExtent2D						vk_swap_chain_extent;
std::vector<VkImageView>		vk_swap_chain_image_views;
VkRenderPass					vk_render_pass;
VkPipelineLayout				vk_pipeline_layout;
VkPipeline						vk_pipeline;
std::vector<VkFramebuffer>		vk_swap_chain_framebuffers;
VkCommandPool					vk_command_pool;
std::vector<VkCommandBuffer>	vk_command_buffers;

VkDebugUtilsMessengerEXT vk_debug_messenger;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	printf("[Vulkan] %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void vk_set_debug_utils_messenger_create_info_ext (VkDebugUtilsMessengerCreateInfoEXT* info) {
	info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info->pfnUserCallback = vk_debug_callback;
	info->pUserData = nullptr;
}

VkResult vk_create_debug_utils_messenger_ext () {
	VkDebugUtilsMessengerCreateInfoEXT info = {};
	vk_set_debug_utils_messenger_create_info_ext(&info);

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT");
	if (!func) return VK_ERROR_EXTENSION_NOT_PRESENT;

	return func(vk_instance, &info, nullptr, &vk_debug_messenger);
}
void vk_destroy_debug_utils_messenger_ext () {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!func) return;
	
	func(vk_instance, vk_debug_messenger, nullptr);
}

void vk_create_instance () {
	// Check extensions
	uint32_t avail_extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &avail_extension_count, nullptr);

	std::vector<VkExtensionProperties> avail_extensions (avail_extension_count);
	if (avail_extension_count > 0)
		vkEnumerateInstanceExtensionProperties(nullptr, &avail_extension_count, avail_extensions.data());

	uint32_t request_extension_count = 0;
	auto request_extension_names = glfwGetRequiredInstanceExtensions(&request_extension_count);
	std::vector<char const*> request_extensions (request_extension_names, request_extension_names + request_extension_count);

	// Check validation layers
	if (vk_enable_validation_layers) {

		request_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		uint32_t layers_count = 0;
		vkEnumerateInstanceLayerProperties(&layers_count, nullptr);

		std::vector<VkLayerProperties> avail_layers(layers_count);
		vkEnumerateInstanceLayerProperties(&layers_count, avail_layers.data());

		for (auto req : vk_validation_layers) {
			for (auto avail : avail_layers) {
				if (strcmp(req, avail.layerName) == 0) {
					vk_layers.push_back(req);
					break;
				}
			}
		}
	}

	// Create instance
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1,0,0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app_info;

	info.enabledExtensionCount	= (uint32_t)request_extensions.size();
	info.ppEnabledExtensionNames = request_extensions.data();

	info.enabledLayerCount	= (uint32_t)vk_layers.size();
	info.ppEnabledLayerNames = vk_layers.data();

	VkDebugUtilsMessengerCreateInfoEXT dbg_create_info = {};
	if (vk_enable_validation_layers) {
		vk_set_debug_utils_messenger_create_info_ext(&dbg_create_info);
		info.pNext = &dbg_create_info;
	}

	VkResult res = vkCreateInstance(&info, nullptr, &vk_instance);
	assert(res == VK_SUCCESS);
}

struct VulkanQueuesFamilies {
	uint32_t graphics_family = 0;
	bool has_graphics_family = false;

	uint32_t present_family = 0;
	bool has_present_family = false;
};

VulkanQueuesFamilies vk_get_queue_families (VkPhysicalDevice dev) {
	VulkanQueuesFamilies families;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families.data());

	int i = 0;
	for (auto& fam : queue_families) {

		if (!families.has_graphics_family && fam.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			families.graphics_family = i;
			families.has_graphics_family = true;
		}

		VkBool32 preset_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, vk_surface, &preset_support);

		// TODO: this would pick the first queue famility that has present support, in my case that are thr graphics and compute queues, should we make sure to activly alway pick the graphics queue here?
		if (!families.has_present_family && preset_support) {
			families.present_family = i;
			families.has_present_family = true;
		}

		i++;
	}

	return families;
}

bool vk_check_device_extensions (VkPhysicalDevice dev) {
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> avail_extensions(extension_count);
	if (extension_count > 0)
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extension_count, avail_extensions.data());

	// Check if any of the desired extensions is not suppored
	bool all_found = true;
	for (auto ext : vk_device_extensions) {
		bool found = false;
		for (auto avail : avail_extensions) {
			if (strcmp(ext, avail.extensionName) == 0) {
				found = true;
				break;
			}
		}
		all_found = all_found && found;
	}

	return all_found;
}

struct VulkanSwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR caps;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

VulkanSwapChainSupportDetails vk_query_swap_chain_support (VkPhysicalDevice dev) {
	VulkanSwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, vk_surface, &details.caps);

	uint32_t formats_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev, vk_surface, &formats_count, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(formats_count);
	if (formats_count > 0)
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, vk_surface, &formats_count, formats.data());
	details.formats = std::move(formats);

	uint32_t present_modes_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(dev, vk_surface, &present_modes_count, nullptr);

	std::vector<VkPresentModeKHR> present_modes(present_modes_count);
	if (present_modes_count > 0)
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, vk_surface, &present_modes_count, present_modes.data());
	details.present_modes = std::move(present_modes);

	return details;
}

void vk_select_device () {

	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

	std::vector<VkPhysicalDevice> devices(device_count);
	if (device_count > 0)
		vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());

	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	bool discrete_gpu;
	VulkanQueuesFamilies families;

	for (auto& dev : devices) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(dev, &props);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(dev, &features);

		auto q_families = vk_get_queue_families(dev);
		if (!q_families.has_graphics_family) continue;
		if (!q_families.has_present_family) continue;

		if (!vk_check_device_extensions(dev)) continue;

		auto swap_chain_support = vk_query_swap_chain_support(dev);
		if (swap_chain_support.formats.size() == 0 || swap_chain_support.present_modes.size() == 0) continue;

		bool is_discrete = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		// pick first suitable discrete gpu or fallback to first suitable integrated gpu
		if (gpu == VK_NULL_HANDLE || (!discrete_gpu && is_discrete)) {
			gpu = dev;
			discrete_gpu = is_discrete;
		}
	}


	vk_physical_device = gpu;
	assert(vk_physical_device != VK_NULL_HANDLE);
}

void vk_create_logical_device () {
	auto q_families = vk_get_queue_families(vk_physical_device);

	float q_prio = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> q_infos;
	uint32_t queues[] = { q_families.graphics_family, q_families.present_family };

	for (auto fam : queues) {
		// We pretend that the present queue is a seperate queue, even though it is usually just the graphics queue
		// make sure that we do not create the same queue twice
		bool duplicate_queue = false;
		for (auto& qi : q_infos) {
			if (qi.queueFamilyIndex == fam) {
				duplicate_queue = true;
				break;
			}
		}
		if (duplicate_queue) continue;

		VkDeviceQueueCreateInfo q_info = {};
		q_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		q_info.queueFamilyIndex = fam;
		q_info.queueCount = 1;
		q_info.pQueuePriorities = &q_prio;

		q_infos.push_back(q_info);
	}

	VkPhysicalDeviceFeatures features = {};

	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.queueCreateInfoCount = (uint32_t)q_infos.size();
	info.pQueueCreateInfos = q_infos.size() > 0 ? q_infos.data() : nullptr;

	info.pEnabledFeatures = &features;

	info.enabledExtensionCount = (uint32_t)vk_device_extensions.size();
	info.ppEnabledExtensionNames = vk_device_extensions.data();

	info.enabledLayerCount	= (uint32_t)vk_layers.size();
	info.ppEnabledLayerNames = vk_layers.data();

	VkResult res = vkCreateDevice(vk_physical_device, &info, nullptr, &vk_device);
	assert(res == VK_SUCCESS);

	vkGetDeviceQueue(vk_device, q_families.graphics_family, 0, &vk_graphics_queue);
	vkGetDeviceQueue(vk_device, q_families.present_family, 0, &vk_present_queue);
}

VkSurfaceFormatKHR vk_choose_swap_surface_format (std::vector<VkSurfaceFormatKHR> const& formats) {
	for (auto& form : formats) {
		// TODO: it seems my gpu supports 16 bit color and hdr color -> test 16 bit with a gradient for banding
		if (form.format == VK_FORMAT_B8G8R8A8_SRGB && form.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return form;
		}
	}

	return formats[0];
}

VkPresentModeKHR vk_choose_swap_present_mode (std::vector<VkPresentModeKHR> const& present_modes) {
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vk_choose_swap_extent (VkSurfaceCapabilitiesKHR const& caps) {
	if (caps.currentExtent.width != UINT32_MAX) {
		return caps.currentExtent;
	} else {
		VkExtent2D ext;
		ext.width  = clamp(window_size.x, caps.minImageExtent.width , caps.maxImageExtent.width );
		ext.height = clamp(window_size.y, caps.minImageExtent.height, caps.maxImageExtent.height);
		return ext;
	}
}

void vk_create_swap_chain () {
	auto support = vk_query_swap_chain_support(vk_physical_device);

	auto format = vk_choose_swap_surface_format(support.formats);
	auto present_mode = vk_choose_swap_present_mode(support.present_modes);
	auto extent = vk_choose_swap_extent(support.caps);

	// TODO: minImageCount is 2, if we want lowest latency possible do we really want to do +1 here? tutorial claims that you would have to wait sometimes
	int image_count = support.caps.minImageCount + 1;
	if (support.caps.maxImageCount > 0)
		image_count = min(image_count, support.caps.maxImageCount);

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = vk_surface;
	info.minImageCount = image_count;
	info.imageFormat = format.format;
	info.imageColorSpace = format.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// TODO: really call this a third time? why not just store it?
	auto q_families = vk_get_queue_families(vk_physical_device);
	
	// TODO: why not just use VK_SHARING_MODE_EXCLUSIVE always and just do the ownership transfer if that has the best performance, is it too complicated?
	// why even support seperate present queue, do they actually occur in practice?
	uint32_t q_family_indices[] = { q_families.graphics_family, q_families.present_family };
	if (q_families.graphics_family != q_families.present_family) {
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = q_family_indices;
	} else {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = nullptr;
	}

	info.preTransform = support.caps.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR(vk_device, &info, nullptr, &vk_swap_chain);
	assert(res == VK_SUCCESS);

	// get images
	uint32_t count = 0;
	vkGetSwapchainImagesKHR(vk_device, vk_swap_chain, &count, nullptr);

	std::vector<VkImage> swap_chain_images(count);
	if (count > 0)
		vkGetSwapchainImagesKHR(vk_device, vk_swap_chain, &count, swap_chain_images.data());

	vk_swap_chain_images = std::move(swap_chain_images);

	// 
	vk_swap_chain_image_format = format.format;
	vk_swap_chain_extent = extent;
}

void vk_create_image_views () {
	vk_swap_chain_image_views.resize(vk_swap_chain_images.size());

	for (size_t i=0; i<vk_swap_chain_images.size(); ++i) {
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = vk_swap_chain_images[i];
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = vk_swap_chain_image_format;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		VkResult res = vkCreateImageView(vk_device, &info, nullptr, &vk_swap_chain_image_views[i]);
		assert(res == VK_SUCCESS);
	}
}

void vk_create_render_pass () {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = vk_swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency depen = {};
	depen.srcSubpass = VK_SUBPASS_EXTERNAL;
	depen.dstSubpass = 0;
	depen.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	depen.srcAccessMask = 0;
	depen.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	depen.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &color_attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &depen;

	VkResult res = vkCreateRenderPass(vk_device, &info, nullptr, &vk_render_pass);
	assert(res == VK_SUCCESS);
}

VkShaderModule vk_create_shader_module (char const* filename) {
	uint64_t size;
	auto data = kiss::load_binary_file(filename, &size);

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = size;
	info.pCode = (uint32_t*)data.get(); // The tutorial talks about how we need to be careful about the memory alignment here, but it should be safe to assume that all allocations are 4 byte aligned
	
	VkShaderModule shader;
	VkResult res = vkCreateShaderModule(vk_device, &info, nullptr, &shader);
	assert(res == VK_SUCCESS);

	return shader;
}

void vk_create_graphics_pipeline () {

	auto vert_module = vk_create_shader_module("shaders/shader.vert.spv");
	auto frag_module = vk_create_shader_module("shaders/shader.frag.spv");

	VkPipelineShaderStageCreateInfo shader_stages[2] = {};

	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = vert_module;
	shader_stages[0].pName = "main";

	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = frag_module;
	shader_stages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vert_input = {};
	vert_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vert_input.vertexBindingDescriptionCount = 0;
	vert_input.pVertexBindingDescriptions = nullptr;
	vert_input.vertexAttributeDescriptionCount = 0;
	vert_input.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width  = (float)vk_swap_chain_extent.width ;
	viewport.height = (float)vk_swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vk_swap_chain_extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = nullptr;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	VkResult res = vkCreatePipelineLayout(vk_device, &pipeline_layout_info, nullptr, &vk_pipeline_layout);
	assert(res == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.stageCount				= 2;
	info.pStages				= shader_stages;
	info.pVertexInputState		= &vert_input;
	info.pInputAssemblyState	= &input_assembly;
	info.pViewportState			= &viewport_state;
	info.pRasterizationState	= &rasterizer;
	info.pMultisampleState		= &multisampling;
	info.pDepthStencilState		= nullptr;
	info.pColorBlendState		= &color_blending;
	info.pDynamicState			= nullptr;
	info.layout					= vk_pipeline_layout;
	info.renderPass				= vk_render_pass;
	info.subpass				= 0;
	info.basePipelineHandle		= VK_NULL_HANDLE;
	info.basePipelineIndex		= -1;

	res = vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &info, nullptr, &vk_pipeline);
	assert(res == VK_SUCCESS);

	vkDestroyShaderModule(vk_device, vert_module, nullptr);
	vkDestroyShaderModule(vk_device, frag_module, nullptr);
}

void vk_create_framebuffers () {
	vk_swap_chain_framebuffers.resize(vk_swap_chain_image_views.size());

	for (size_t i=0; i<vk_swap_chain_image_views.size(); ++i) {
		VkImageView attachments[] = {
			vk_swap_chain_image_views[i],
		};

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = vk_render_pass;
		info.attachmentCount = 1;
		info.pAttachments = attachments;
		info.width  = vk_swap_chain_extent.width ;
		info.height = vk_swap_chain_extent.height;
		info.layers = 1;

		VkResult res = vkCreateFramebuffer(vk_device, &info, nullptr, &vk_swap_chain_framebuffers[i]);
		assert(res == VK_SUCCESS);
	}

}

void vk_create_command_pool () {
	auto q_families = vk_get_queue_families(vk_physical_device); // TODO: again?

	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = q_families.graphics_family;
	info.flags = 0;

	VkResult res = vkCreateCommandPool(vk_device, &info, nullptr, &vk_command_pool);
	assert(res == VK_SUCCESS);
}

void vk_create_command_buffers () {
	vk_command_buffers.resize(vk_swap_chain_image_views.size());

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = vk_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount  = (uint32_t)vk_command_buffers.size();

	VkResult res = vkAllocateCommandBuffers(vk_device, &alloc_info, vk_command_buffers.data());
	assert(res == VK_SUCCESS);

	for (size_t i=0; i<vk_command_buffers.size(); ++i) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		res = vkBeginCommandBuffer(vk_command_buffers[i], &begin_info);
		assert(res == VK_SUCCESS);

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = vk_render_pass;
		render_pass_info.framebuffer = vk_swap_chain_framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = vk_swap_chain_extent;

		VkClearValue clear_color = { 0, 0, 0, 1 };
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(vk_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(vk_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

		vkCmdDraw(vk_command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(vk_command_buffers[i]);

		res = vkEndCommandBuffer(vk_command_buffers[i]);
		assert(res == VK_SUCCESS);
	}
}

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;

void vk_create_semaphores () {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(vk_swap_chain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkResult res1 = vkCreateSemaphore(vk_device, &info, nullptr, &imageAvailableSemaphores[i]);
		VkResult res2 = vkCreateSemaphore(vk_device, &info, nullptr, &renderFinishedSemaphores[i]);
		VkResult res3 = vkCreateFence(vk_device, &fence_info, nullptr, &inFlightFences[i]);
		assert(res1 == VK_SUCCESS && res2 == VK_SUCCESS && res3 == VK_SUCCESS);
	}
}

void vk_init () {
	vk_create_instance();

	if (vk_enable_validation_layers)
		vk_create_debug_utils_messenger_ext();

	auto res = glfwCreateWindowSurface(vk_instance, glfw_window, nullptr, &vk_surface);
	assert(res == VK_SUCCESS);

	vk_select_device();
	vk_create_logical_device();
	vk_create_swap_chain();
	vk_create_image_views();
	vk_create_render_pass();
	vk_create_graphics_pipeline();
	vk_create_framebuffers();
	vk_create_command_pool();
	vk_create_command_buffers();
	vk_create_semaphores();
}

void vk_deinit () {
	vkDeviceWaitIdle(vk_device);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(vk_device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(vk_device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(vk_device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);

	for (auto& fb : vk_swap_chain_framebuffers) {
		vkDestroyFramebuffer(vk_device, fb, nullptr);
	}

	vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
	vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);
	vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

	for (auto& iv : vk_swap_chain_image_views)
		vkDestroyImageView(vk_device, iv, nullptr);

	vkDestroySwapchainKHR(vk_device, vk_swap_chain, nullptr);
	vkDestroyDevice(vk_device, nullptr);

	if (vk_enable_validation_layers)
		vk_destroy_debug_utils_messenger_ext();

	vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
	vkDestroyInstance(vk_instance, nullptr);
}

size_t currentFrame = 0;

void draw () {
	vkWaitForFences(vk_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	
	// Aquire image
	uint32_t image_index;
	vkAcquireNextImageKHR(vk_device, vk_swap_chain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &image_index);

	if (imagesInFlight[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(vk_device, 1, &imagesInFlight[image_index], VK_TRUE, UINT64_MAX);
	}

	imagesInFlight[image_index] = inFlightFences[currentFrame];

	// Draw image
	VkSemaphore wait_semaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore signal_semaphores[] = { renderFinishedSemaphores[currentFrame] };

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vk_command_buffers[image_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(vk_device, 1, &inFlightFences[currentFrame]);

	VkResult res = vkQueueSubmit(vk_graphics_queue, 1, &submit_info, inFlightFences[currentFrame]);
	assert(res == VK_SUCCESS);

	// Present image
	VkSwapchainKHR swap_chains[] = { vk_swap_chain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;

	vkQueuePresentKHR(vk_present_queue, &present_info);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main () {

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfw_window = glfwCreateWindow(window_size.x, window_size.y, "Vulkan window", nullptr, nullptr);

	vk_init();

	while(!glfwWindowShouldClose(glfw_window)) {
		glfwPollEvents();

		draw();
	}

	vk_deinit();

	glfwDestroyWindow(glfw_window);

	glfwTerminate();

	return 0;
}
