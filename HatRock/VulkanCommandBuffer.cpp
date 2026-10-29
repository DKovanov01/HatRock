#include "VulkanRenderer.h"

void VulkanRenderer::CreateCommandPool() { // Create a command pool for allocating command buffers
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; // this Vulkan command means that we are creating a command pool, which is a pool of memory that can be used to allocate command buffers. Command buffers are used to record rendering commands that will be submitted to the GPU for execution.
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // this means that the command pool will be created for the graphics queue family, which is the queue that will be used to submit rendering commands.
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // this means that the command buffers allocated from this pool can be reset individually, which is useful for re-recording commands without having to destroy and recreate the command buffer.

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &primaryCommandBuffer.commandPool) != VK_SUCCESS) { // this function creates the command pool and returns a VkResult value that indicates whether the operation was successful or not. 
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void VulkanRenderer::CreateCommandBuffers() {
	primaryCommandBuffer.commandBuffers.resize(numSwapchains); // create X number for however many swap chains we havere, size the command buffer vector to hold one command buffer for each swapchain image

	VkCommandBufferAllocateInfo allocInfo{}; // this struct is used to specify the parameters for allocating command buffers from a command pool
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // this means that we are allocating command buffers, which are used to record rendering commands that will be submitted to the GPU for execution.
	allocInfo.commandPool = primaryCommandBuffer.commandPool; // this means that the command buffers will be allocated from the command pool that we created earlier, which is associated with the graphics queue family.
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // this means that the command buffers will be primary command buffers, which can be submitted directly to the graphics queue for execution. Secondary command buffers can only be called from primary command buffers.
	allocInfo.commandBufferCount = (uint32_t)primaryCommandBuffer.commandBuffers.size(); // this means that we want to allocate as many command buffers as there are swapchain images, which is the number of frames that can be rendered simultaneously.

	if (vkAllocateCommandBuffers(device, &allocInfo, primaryCommandBuffer.commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanRenderer::RecordCommandBuffers(Recording start_stop) {
	if (start_stop == Recording::START) {
		vkDeviceWaitIdle(device);
		for (size_t i = 0; i < primaryCommandBuffer.commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

				if (vkBeginCommandBuffer(primaryCommandBuffer.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
					throw std::runtime_error("failed to begin recording command buffer!");
				}

			VkRenderPassBeginInfo renderPassInfo{}; // fill out some forms		
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i]; // where is it? swap chains. Loop over them.
			renderPassInfo.renderArea.offset = { 0, 0 }; // where to start drawing? top left corner
			renderPassInfo.renderArea.extent = swapChainExtent; // how big is the area? same size as the swap chain

			std::array<VkClearValue, 2>clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // set the color for something other than black? change these values
			clearValues[1].depthStencil = { 1.0f, 0 }; // set the depth for something other than black? change these values

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(primaryCommandBuffer.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // main call to begin the render pass
		}
	}
	else if (start_stop == Recording::STOP) {
		for (size_t i = 0; i < primaryCommandBuffer.commandBuffers.size(); i++) {
			vkCmdEndRenderPass(primaryCommandBuffer.commandBuffers[i]); // main call to end the render pass
			if (vkEndCommandBuffer(primaryCommandBuffer.commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
}

void VulkanRenderer::DestroyCommandBuffers() {
	vkFreeCommandBuffers(device, primaryCommandBuffer.commandPool, 1, primaryCommandBuffer.commandBuffers.data()); // Instead of 1 it could be "static_cast<uint32_t>(primaryCommandBuffer.commandBuffers.size())", but we only have one command buffer in this case. This function frees the command buffers that were allocated from the command pool, which is necessary to avoid memory leaks and ensure proper cleanup of Vulkan resources.
	vkDestroyCommandPool(device, primaryCommandBuffer.commandPool, nullptr);
}