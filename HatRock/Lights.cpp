#include "Lights.h"

bool Lights::OnCreate(VulkanRenderer* vRenderer) {

	lightsUBO = vRenderer->CreateUniformBuffers<LightsData>(); // data about lights | Make a UBO for each member of the chain

	lights.diffuse[0] = Vec4(0.0, 0.0, 0.9, 0.0);
	lights.specular[0] = Vec4(0.0, 0.3, 0.0, 0.0);
	lights.ambient = Vec4(0.1, 0.1, 0.1, 0.0);
	lights.pos[0] = Vec4(-2.0f, 0.0f, -5.0f, 0.0f);
	lights.numLights = 1;

	lights.diffuse[1] = Vec4(0.9, 0.0, 0.0, 0.0);
	lights.specular[1] = Vec4(0.0, 0.3, 0.0, 0.0);
	lights.pos[1] = Vec4(2.0f, 0.0f, -5.0f, 0.0f);
	lights.numLights = 2;

	lights.diffuse[2] = Vec4(0.0, 0.9, 0.0, 0.0);
	lights.specular[2] = Vec4(0.0, 0.3, 0.0, 0.0);
	lights.pos[2] = Vec4(0.0f, 2.0f, -5.0f, 0.0f);
	lights.numLights = 3;

	vRenderer->UpdateUniformBuffer<LightsData>(lights, lightsUBO); // just write some data into the uniformBuffer | what kind/ a LightsData kind; Then pass it the vector & lights data
	// how does it know it nseeds lights and lightsUBO? Because it's a templated function
	//vRenderer->UpdateUniformBuffer<CameraData>(camera, cameraUBO); // same

	return true;
}

