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

		mario = registry.create(); // EnTT's own create
		registry.emplace<TransformComponent>(mario); // Each emplace creates a component and associates it with Mario.
		auto& marioTransform = registry.get<TransformComponent>(mario); // Gets the component
		marioTransform.position = Vec3(0.0f, 0.0f, 0.0f); // Assigns a position to the component
		marioTransform.rotationMatrix = MMath::rotate(0.0f, Vec3(0.0f, 1.0f, 0.0f)); // Assigns a rotation to the component

		registry.emplace<MeshComponent>(mario); // Associate mesh component with Mario;
		auto& marioMesh = registry.get<MeshComponent>(mario); // Get the mesh component
		marioMesh.mesh = vRenderer->LoadModelIndexed("./Assets/meshes/Mario.obj"); // Assign the mesh to the component

		registry.emplace<TextureComponent>(mario); // Associate shader component with Mario
		auto& marioTexture = registry.get<TextureComponent>(mario); // Get the shader component
		marioTexture.texture = vRenderer->Create2DTextureImage("./Assets/textures/mario_fire.png"); // Assign the texture to the component
		DescriptorSetBuilder marioDescriptorBuilder(vRenderer->getDevice()); // Create a descriptor set builder for Mario's shader component

		registry.emplace<ShaderComponent>(mario);
		auto& marioShader = registry.get<ShaderComponent>(mario); // Get the shader component
		marioDescriptorBuilder.add(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &marioTexture.texture); // Add the texture to the descriptor set builder
		marioShader.descriptorSetInfo = marioDescriptorBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains()); // Build the descriptor set and assign it to the shader component

		//registry.emplace<PhysicsComponent>(mario);
		//registry.emplace<CollisionComponent>(mario);

		luigi = registry.create();

		registry.emplace<TransformComponent>(luigi);
		auto& luigiTransform = registry.get<TransformComponent>(luigi);
		luigiTransform.position = Vec3(2.0f, 0.0f, 0.0f);
		luigiTransform.rotationMatrix = MMath::rotate(180.0f, Vec3(0.0f, 1.0f, 0.0f));

		registry.emplace<MeshComponent>(luigi);
		auto& luigiMesh = registry.get<MeshComponent>(luigi);
		luigiMesh.mesh = vRenderer->LoadModelIndexed("./Assets/meshes/Mario.obj");

		registry.emplace<TextureComponent>(luigi);
		auto& luigiTexture = registry.get<TextureComponent>(luigi);
		luigiTexture.texture = vRenderer->Create2DTextureImage("./Assets/textures/mario_mime.png");
		DescriptorSetBuilder luigiDescriptorBuilder(vRenderer->getDevice());

		registry.emplace<ShaderComponent>(luigi);
		auto& luigiShader = registry.get<ShaderComponent>(luigi); // Get the shader component
		luigiDescriptorBuilder.add(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &luigiTexture.texture);
		luigiShader.descriptorSetInfo =	luigiDescriptorBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains());

		//registry.emplace<PhysicsComponent>(luigi);
		//registry.emplace<CollisionComponent>(luigi);

		{ // set 0 
			DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice()); // get the device from the renderer, and pass it to the descriptorSetBuilder constructor
			descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getCameraUBO()); // add a descriptor to the builder, binding point 0, type uniform buffer, stage flags vertex shader, count 1, and pass the camera UBO
			descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lights->getLightsUBO()); // add another descriptor to the builder, binding point 1, type uniform buffer, stage flags vertex shader and fragment shader, count 1, and pass the lights UBO
			cameraDescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains()); // build the descriptor set, and pass the number of swapchains to the builder
		}

		//actorDescriptorSetLayoutArray.push_back(cameraDescriptorSetInfo.descriptorSetLayout);
		//actorDescriptorSetLayoutArray.push_back(actors[0]->getDescriptorSetInfo().descriptorSetLayout);
		actorDescriptorSetLayoutArray.push_back(cameraDescriptorSetInfo.descriptorSetLayout);
		actorDescriptorSetLayoutArray.push_back(registry.get<ShaderComponent>(mario).descriptorSetInfo.descriptorSetLayout);

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
	/*mario->SetRotation(rotation);
	luigi->SetRotation(180);
	for (Actor* actor : actors) {
		actor->Update(deltaTime);
	}*/

	auto& marioTransform = registry.get<TransformComponent>(mario);
	marioTransform.rotationMatrix =	MMath::rotate(rotation, Vec3(0.0f, 1.0f, 0.0f));
	auto view = registry.view<TransformComponent, PhysicsComponent>();

	for (auto entity : view) {
		auto& transform = view.get<TransformComponent>(entity);
		auto& physics = view.get<PhysicsComponent>(entity);

		transform.position += physics.velocity * deltaTime;
		physics.velocity += physics.acceleration * deltaTime;

		transform.modelMatrix = MMath::translate(transform.position) * transform.rotationMatrix;
	}

	marioTransform.rotationMatrix =	MMath::rotate(rotation, Vec3(0.0f, 1.0f, 0.0f));
	TransformSystem::Update(registry, deltaTime);
}

void Scene0::Render() const {
	switch (renderer->getRendererType()) {

	case RendererType::VULKAN: {
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);


		vRenderer->RecordCommandBuffers(Recording::START); // tell the commandBuffers (plural, keep that in mind) to start recording; Then send it a bunch of commands
		vRenderer->BindPipeline(pipelineInfo.pipeline); // go bind the pipeline
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, cameraDescriptorSetInfo.descriptorSet, 0); // next go bind a descriptor set(s) to the pipeline layout; This is a vector of descriptor sets, so we can bind more than 1 at a time
		/*for (Actor* actor : actors) {
			actor->Render(vRenderer, pipelineInfo);
		}*/
		auto view = registry.view<TransformComponent, MeshComponent, ShaderComponent>();
		for (auto entity : view) {
			auto& transform = view.get<TransformComponent>(entity);
			auto& mesh = view.get<MeshComponent>(entity);
			auto& shader = view.get<ShaderComponent>(entity);
			vRenderer->BindMesh(mesh.mesh);
			vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, shader.descriptorSetInfo.descriptorSet, 1);
			vRenderer->SetPushConstant(pipelineInfo, transform.modelMatrix);
			vRenderer->DrawIndexed(mesh.mesh);
		}
		vRenderer->RecordCommandBuffers(Recording::STOP); // Then turn the recorder off and submit it
		vRenderer->Render();
		break;
	}

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
		/*for (Actor* actor : actors) {
			actor->OnDestroy(vRenderer);
			delete actor;
		}
		actors.clear();*/
		// Destroy entity-specific shader resources
		auto shaderView = registry.view<ShaderComponent>();

		//for (auto entity : shaderView) {
		//	auto& shader = shaderView.get<ShaderComponent>(entity);
		//	vRenderer->DestroyDescriptorSet(shader.descriptorSetInfo);
		//	vRenderer->DestroySampler2D(shader.texture);
		//}

		// Destroy entity-specific meshes
		auto meshView = registry.view<MeshComponent>();
		for (auto entity : meshView) {
			auto& mesh = meshView.get<MeshComponent>(entity);
			vRenderer->DestroyIndexedMesh(mesh.mesh);
		}

		// Remove all entities and their components
		registry.clear();
		vRenderer->DestroyDescriptorSet(cameraDescriptorSetInfo);
		vRenderer->DestroyUBO(lights->getLightsUBO());
		vRenderer->DestroyUBO(camera->getCameraUBO()); // dis
	}


}

