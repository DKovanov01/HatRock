#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene0.h"
#include <MMath.h>
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"

Scene0::Scene0(Renderer* renderer_) :
	Scene(nullptr), renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0::~Scene0() {
}

bool Scene0::OnCreate() {
	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(vRenderer->getPhysicalDevice(), &deviceProperties); // !!!! possibly make this a Singleton !!!!

		uint32_t maxPushConstantSize = deviceProperties.limits.maxPushConstantsSize;

		std::cout << "Max Push Constant Size: " << maxPushConstantSize << " bytes" << std::endl;

		camera = new Camera();
		camera->OnCreate(vRenderer);

		lights = new Lights();
		lights->OnCreate(vRenderer);

		mario = new Actor();
		mario->SetMesh(vRenderer->LoadModelIndexed("./Assets/meshes/Mario.obj"));
		mario->SetTexture(vRenderer->Create2DTextureImage("./Assets/textures/mario_fire.png"));
		mario->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
		mario->OnCreate(vRenderer);
		actors.push_back(mario);

		luigi = new Actor();
		luigi->SetMesh(vRenderer->LoadModelIndexed("./Assets/meshes/Mario.obj"));
		luigi->SetTexture(vRenderer->Create2DTextureImage("./Assets/textures/mario_mime.png"));
		luigi->SetPosition(Vec3(2.0f, 0.0f, 0.0f));
		luigi->SetOrientation(Quaternion(90.0f, Vec3(0.0f, 0.0f, 0.0f)));
		luigi->OnCreate(vRenderer);
		actors.push_back(luigi);

		{ // set 0 
			DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice()); // get the device from the renderer, and pass it to the descriptorSetBuilder constructor
			descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getCameraUBO()); // add a descriptor to the builder, binding point 0, type uniform buffer, stage flags vertex shader, count 1, and pass the camera UBO
			descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lights->getLightsUBO()); // add another descriptor to the builder, binding point 1, type uniform buffer, stage flags vertex shader and fragment shader, count 1, and pass the lights UBO
			cameraDescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains()); // build the descriptor set, and pass the number of swapchains to the builder
		}

		actorDescriptorSetLayoutArray.push_back(cameraDescriptorSetInfo.descriptorSetLayout);
		actorDescriptorSetLayoutArray.push_back(actors[0]->getDescriptorSetInfo().descriptorSetLayout);

		pipelineInfo = vRenderer->CreateGraphicsPipeline(actorDescriptorSetLayoutArray, "./Shaders/multiPhong.vert.spv", "./Shaders/multiPhong.frag.spv"); // need to recompile these for assignment 1

	}
	break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) {

	switch (sdlEvent.type) {
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
		float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
		///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
		if (renderer->getRendererType() == RendererType::VULKAN) {
			dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
		}
		break;
	}

}
void Scene0::Update(const float deltaTime) {
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;
	rotation += 90.0f * deltaTime;
	mario->SetRotation(rotation);
	luigi->SetRotation(180);
	for (Actor* actor : actors) {
		actor->Update(deltaTime);
	}
}

void Scene0::Render() const {
	switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);


		vRenderer->RecordCommandBuffers(Recording::START); // tell the commandBuffers (plural, keep that in mind) to start recording; Then send it a bunch of commands
		vRenderer->BindPipeline(pipelineInfo.pipeline); // go bind the pipeline
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, cameraDescriptorSetInfo.descriptorSet, 0); // next go bind a descriptor set(s) to the pipeline layout; This is a vector of descriptor sets, so we can bind more than 1 at a time
		for (Actor* actor : actors) {
			actor->Render(vRenderer, pipelineInfo);
		}
		vRenderer->RecordCommandBuffers(Recording::STOP); // Then turn the recorder off and submit it
		vRenderer->Render();
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);

		break;
	}
}


void Scene0::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if (vRenderer) {
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyCommandBuffers();
		vRenderer->DestroyPipeline(pipelineInfo);
		for (Actor* actor : actors) {
			actor->OnDestroy(vRenderer);
			delete actor;
		}
		actors.clear();
		vRenderer->DestroyDescriptorSet(cameraDescriptorSetInfo);
		vRenderer->DestroyUBO(lights->getLightsUBO());
		vRenderer->DestroyUBO(camera->getCameraUBO()); // dis
	}


}

