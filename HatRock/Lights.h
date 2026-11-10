#pragma once
#include "Renderer.h"
#include "VulkanRenderer.h"
#include "Vector.h"
#include "CoreStructs.h"
using namespace MATH;

class Lights {
private:
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;

public:
	bool OnCreate(VulkanRenderer* renderer);
	//LightsData getLights() { return lights; }
	std::vector<BufferMemory> getLightsUBO() { return lightsUBO; }
};
