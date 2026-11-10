#ifndef SCENE0_H
#define SCENE0_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
#include "Camera.h"
#include "Lights.h"
#include "CoreStructs.h"
#include "Actor.h"

using namespace MATH;

/// Forward declarations 
union SDL_Event;


class Scene0 : public Scene {
private:

	Renderer* renderer;
	Camera* camera;
	Lights* lights;

	std::vector<Actor*> actors;
	DescriptorSetInfo cameraDescriptorSetInfo;
	std::vector<VkDescriptorSetLayout> actorDescriptorSetLayoutArray;

	PipelineInfo pipelineInfo;
	CommandBufferData commandBufferData;

	Actor* mario;
	Actor* luigi;
	float rotation = 0.0f;

public:

	explicit Scene0(Renderer* renderer_);
	virtual ~Scene0();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event& sdlEvent) override;
};


#endif // SCENE0_H