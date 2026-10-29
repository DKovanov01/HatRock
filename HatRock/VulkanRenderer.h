#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3_image/SDL_image.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <unordered_map>
#include <array>
#include <queue>

#include "CoreStructs.h"
#include "Renderer.h"
#include "DescriptorSetBuilder.h"

#include <VMath.h>
#include <MMath.h>
#include <Hash.h>

using namespace MATH;

#ifdef NDEBUG /// only use validation layers if in debug mode
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = { // this is the validation layer that we want to use, which is a standard one provided by Khronos
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME // this means that we want to use the swapchain extension, which is required for rendering to a window
};

struct QueueFamilyIndices {
	/// optional means that it contains no value until it is assigned.
	/// has_value() returns false if no value has ever been assigned.
	std::optional<uint32_t> graphicsFamily; // graphicsFamily is an optional uint32_t that will hold the index of the graphics queue family if it exists.
	std::optional<uint32_t> presentFamily; // presentFamily is an optional uint32_t that will hold the index of the present queue family if it exists.
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value(); // isComplete() returns true if both have values assigned to them.
	}
};

struct Vertex {
	Vec3 pos;
	Vec3 normal;
	Vec2 texCoord;

	/// Used in vertex deduplication. The operator == will compare two vertices and determine if they are exactly identical.
	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription() { // this function returns a VkVertexInputBindingDescription struct that describes how the vertex data is laid out in memory.
		VkVertexInputBindingDescription bindingDescription{}; // this initializes a VkVertexInputBindingDescription struct with default values.
		bindingDescription.binding = 0; // this means that the vertex data is bound to binding 0 in the shader.
		bindingDescription.stride = sizeof(Vertex); // this means that the distance in bytes between each vertex in the buffer is equal to the size of the Vertex struct.
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // this means that the vertex data is per-vertex, not per-instance.
		return bindingDescription; // returns the information we filled out above
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() { // this initializes an array of 3 VkVertexInputAttributeDescription structs with default values.
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{}; // each of the attributes below - location, format, offset for pos/normal/texCoord are at binding 0 in the shader;
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // this means that the position attribute is a vec3 of floats
		attributeDescriptions[0].offset = offsetof(Vertex, pos); // this means that the position attribute is at the offset of the pos member in the Vertex struct
	
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

/// Generate a hash of a Vertex, used in vertex deduplication
/// Addint this to namespace std is called a namespace injection. 
// It is a way to add a function to a namespace that you don't own. 
// In this case, we are adding a hash function for Vertex to the std namespace 
// so that we can use it in an unordered_map. 
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const noexcept {
			size_t hash1 = hash<Vec3>()(vertex.pos);
			size_t hash2 = hash<Vec3>()(vertex.normal);
			size_t hash3 = hash<Vec2>()(vertex.texCoord);
			size_t result = ((hash1 ^ (hash2 << 1)) >> 1) ^ (hash3 << 1); // this combines the three hashes into one hash value
			return result;
		}
	};
}

enum Recording {
	START,
	STOP
};

class VulkanRenderer : public Renderer {
	/// C11 precautions
	VulkanRenderer(const VulkanRenderer&) = delete; /// Copy constructor
	VulkanRenderer(VulkanRenderer&&) = delete;      /// Move constructor
	VulkanRenderer& operator=(const VulkanRenderer&) = delete; /// Copy operator
	VulkanRenderer& operator=(VulkanRenderer&&) = delete;      /// Move operator

public:
	VulkanRenderer();
	~VulkanRenderer();

	SDL_Window* CreateWindow(std::string name_, int width_, int height_);
	bool OnCreate();
	void OnDestroy();
	void Render();
	void RecreateSwapChain(); // this function is called when the window is resized or minimized, and it recreates the swap chain and all the resources that depend on it.

	/// See VulkanSampler.cpp
	Sampler2D Create2DTextureImage(const char* textureFile);
	void createTextureImageView(Sampler2D& texture2D);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void createTextureSampler(Sampler2D& texture2D) const;
	void DestroySampler2D(Sampler2D& texture2D);

	/// See DescriptorSetBuilder.cpp
	void BindDescriptorSet(VkPipelineLayout pipelineLayout, const std::vector<VkDescriptorSet>& descriptorSet, uint32_t firstSet);
	void DestroyDescriptorSet(DescriptorSetInfo& descriptorSetInfo);

	/// See VulkanPipeline.cpp
	PipelineInfo CreateGraphicsPipeline(std::vector<VkDescriptorSetLayout> descriptorSetLayoutArray, const char* vertFile, const char* fragFile,
		const char* tessCtrlFile = nullptr, const char* tessEvalFile = nullptr, const char* geomFile = nullptr);
	void BindPipeline(VkPipeline pipeline);
	void DestroyPipeline(PipelineInfo pipelineInfo);
	static std::vector<char> readFile(const std::string& fileName);

	/// See VulkanMesh.cpp
	IndexedVertexBuffer LoadModelIndexed(const char* filename);
	void BindMesh(IndexedVertexBuffer mesh);
	void DrawIndexed(IndexedVertexBuffer mesh);
	void DestroyIndexedMesh(IndexedVertexBuffer mesh_);
private:
	void CreateVertexBuffer(IndexedVertexBuffer& indexedVertexBuffer, const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(IndexedVertexBuffer& indexedVertexBuffer, const std::vector<uint32_t>& indices);
	/// A helper function for createVertexBuffer()
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
	/// See VulkanCommandBuffer.cpp
	void CreateCommandPool();
	void CreateCommandBuffers();
	void DestroyCommandBuffers();
	void RecordCommandBuffers(Recording start_stop);
	void SetPushConstant(const PipelineInfo pipelineInfo, const Matrix4& modelMatrix_);

	/// Some inlined getter and setters
	SDL_Window* getWindow() { return window; }
	VkDevice getDevice() { return device; }
	uint32_t getNumSwapchains() { return numSwapchains; }
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

	template<class T>
	std::vector<BufferMemory> CreateUniformBuffers() {
		std::vector<BufferMemory> ubo; /// build a number of UBOs
		VkDeviceSize bufferSize = sizeof(T);
		ubo.resize(numSwapchains);
		for (size_t i = 0; i < numSwapchains; i++) {
			ubo[i].bufferMemoryLength = bufferSize; // figure out how big the buffer's gonna be 
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,  // create a buffer (no Vk, means Scott built this for us); Turn on 1 bit, say this memory's gonna be used as uniform buffer
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Turn on 2 bits, say this memory's gonna be visible to the host (CPU) and that it is coherent (no caching)
				// Wanna be able to see it ? Yeah | Don't even ask, this is about how memory is optimiezd in VRAM for fast retrieval, is it read only, where is it stored, etc etc
				ubo[i].bufferID, ubo[i].bufferMemoryID);
		}
		return ubo;
	}

	template <class T>
	void UpdateUniformBuffer(const T& scrData, const std::vector<BufferMemory> bufferMemory) {
		void* data;  // good ol' void* meaning idc*; Pointing to what? I don't know, and I don't care. On a 64 bit machine, a void* is an 8bit unsigned int that holds the address of something; What does it point to? I don't care
		for (int i = 0; i < numSwapchains; i++) { // let's loop over all the things in the swapChain
			vkMapMemory(device, bufferMemory[i].bufferMemoryID, 0, sizeof(T), 0, &data); // call vkMapMemory; 0 before the data is "let's start at beginning" AKA offset to beginning; Then &data - the address of a void*
			// even though we can't have the address of anything in the GPU, there's a lil' region where there is memory (loading dock mentioned elsewhere)
			memcpy(data, &scrData, bufferMemory[i].bufferMemoryLength);  // copy memory into that data (void*), pass all the sourceData (light, cam, whatever), how long is it
			vkUnmapMemory(device, bufferMemory[i].bufferMemoryID); // blow up the staging area now that the stuff has been moved
		} // then go do it all for X number of swap chains
	};

	void DestroyUBO(std::vector<BufferMemory> ubo);

private: /// Private member variables
	const uint32_t MAX_FRAMES_IN_FLIGHT;
	uint32_t numSwapchains;
	uint32_t currentFrame;
	SDL_Event sdlEvent;
	uint32_t windowWidth;
	uint32_t windowHeight;
	SDL_Window* window;
	VkInstance instance; // basically a handle to the Vulkan library, which is loaded at runtime. It is used to create and manage Vulkan objects, such as devices, swapchains, and command buffers.
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface; // a surface is an abstraction for a platform-specific window or display that can be used for rendering. It is created from an SDL_Window and is used to present images to the screen.
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkRenderPass renderPass; // a render pass is a collection of attachments, subpasses, and dependencies that define how rendering operations are performed. It is used to create framebuffers and pipelines.
	VkImage depthImage; // a depth image is an image that stores the depth values of the scene, which are used for depth testing and occlusion. It is created with a depth format and a depth attachment.
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkSemaphore> imageAvailableSemaphores; // a semaphore is a synchronization primitive that can be used to signal and wait for events. It is used to synchronize the rendering and presentation of images.
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences; // a fence is another synchronization primitive that can be used to signal and wait for events. It is used to synchronize the CPU and GPU.
	std::vector<VkFence> imagesInFlight; // this is a vector of fences that are used to track which images are currently being rendered. It is used to prevent rendering to the same image multiple times.
	std::vector<VkImage> swapChainImages; // this is a vector of images that are used for rendering. It is created from the swapchain and is used to present images to the screen.
	std::vector<VkImageView> swapChainImageViews; // this is a vector of image views that are used to access the images in the swapchain. It is created from the swapchain images and is used to create framebuffers.
	std::vector<VkFramebuffer> swapChainFramebuffers; // this is a vector of framebuffers that are used for rendering. It is created from the swapchain image views and the render pass.

	VkQueue graphicsQueue; // This queue is used to submit command buffers for rendering. It is created from the physical device and is used to execute rendering commands on the GPU.
	VkQueue presentQueue; // This queue is used to present images to the screen. It is created from the physical device and is used to present images from the swapchain.

	CommandBufferData primaryCommandBuffer{}; // This is a struct that holds the primary command buffer and its associated command pool. The primary command buffer is used to record rendering commands that will be submitted to the graphics queue.
	std::vector<Sampler2D> textures2D;
	std::vector<BufferMemory> cameraUboBuffers;
	std::vector<BufferMemory> lightUboBuffers;
	std::queue<ModelMatrixPushConst> pushConstant; // This is a queue of push constants that are used to update the model matrix in the vertex shader. Push constants are a way to pass small amounts of data to shaders without using uniform buffers.
	std::vector<IndexedVertexBuffer> indexedVertexBuffers;

private:
	bool hasStencilComponent(VkFormat format); // This function checks if a given format has a stencil component. It is used to determine if a depth format can be used for depth testing and stencil testing. 
	// Stencil is a technique used in computer graphics to limit the area of rendering to certain parts of the screen. It is often used for effects like shadows, reflections, and masking.
	void CreateInstance();
	void CreateSurface();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();



	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateRenderPass();
	void CreateFrameBuffers();
	void CreateDepthResources();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer scrBuffer, VkBuffer dstBuffer, VkDeviceSize size); // This function copies data from one buffer to another. It is used to transfer vertex and index data from the CPU to the GPU.



	void createSyncObjects(); // This function creates the synchronization objects (semaphores and fences) that are used to synchronize rendering and presentation.
	void destroySwapChain();

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout); // This function transitions an image from one layout to another. It is used to prepare images for rendering and presentation.
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkFormat findDepthFormat(); // This function finds a suitable depth format for the depth image. It is used to create the depth image and its associated image view.
	// depth image is used for depth testing, which determines which objects are in front of others in 3D space.
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features); // This function finds a supported format from a list of candidates. It is used to find a suitable format for the depth image and other images.
	// formats could be different types of image formats, such as RGBA, RGB, or depth formats. The function checks if the format is supported by the physical device and if it meets the required features.

	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();


	// below are a bunch of checks for whether our GPU is suitable for our needs. We need to check if it supports the features we need, such as graphics and presentation queues, swapchain support, and device extensions.
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();

	VkCommandBuffer beginSingleTimeCommands(); // This function begins recording a single-use command buffer. It is used to record commands that will be executed once and then discarded.
	void endSingleTimeCommands(VkCommandBuffer commandBuffer); // This function ends recording a single-use command buffer and submits it to the graphics queue. It is used to execute commands that were recorded in a single-use command buffer.
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device); // This function finds the queue families that support graphics and presentation. It is used to create the logical device and its associated queues.
	VkShaderModule createShaderModule(const std::vector<char>& code); // This function creates a shader module from SPIR-V bytecode. It is used to create the vertex and fragment shaders for the graphics pipeline.
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); // This function chooses a suitable surface format for the swapchain. It is used to determine how images are stored in the swapchain.
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); // This function chooses a suitable present mode for the swapchain. It is used to determine how images are presented to the screen.
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); // This function chooses a suitable extent (width and height) for the swapchain. It is used to determine the size of the images in the swapchain.
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); // This function queries the swapchain support details for a given physical device. It is used to determine if the device supports the required features for the swapchain.

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData); // This function is a callback that is called by the validation layers when a validation message is generated. It is used to log validation messages to the console.

	/// Used to reformat the Validatoin Layer's output to the console
	static void insertLineBreaks(std::string& str, size_t maxWidth) {
		size_t index = 0;
		while (index + maxWidth < str.size()) {
			/// Find a blank space before the maxWidth is encountered
			size_t breakLocation = str.find(' ', index + maxWidth);
			if (breakLocation == std::string::npos || breakLocation <= index) {
				/// If no blank space, just break at maxWidth
				breakLocation = index + maxWidth;
			}
			str[breakLocation] = '\n'; // Replace the blank space with a newline character
			index = breakLocation + 1;
		}
	}
};

#endif 