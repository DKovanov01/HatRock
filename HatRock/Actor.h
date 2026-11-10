#pragma once
#include <Matrix.h>
#include <Quaternion.h>
#include "DescriptorSetBuilder.h"
#include "VulkanRenderer.h"
#include "CoreStructs.h"

using namespace MATH;

class Actor {
private:
	Vec3 pos;
	Vec3 vel;
	Vec3 accel;
	Quaternion orientation;
	Matrix4 modelMatrix;
	Matrix4 rotationMatrix;

	DescriptorSetInfo descriptorSetInfo;
	//std::vector<BufferMemory> actorData;

	IndexedVertexBuffer mesh;
	Sampler2D texture;

public:
	bool OnCreate(VulkanRenderer* vRenderer);
	void OnDestroy(VulkanRenderer* vRenderer);

	void SetMesh(IndexedVertexBuffer mesh_);
	void SetTexture(Sampler2D texture_);

	void SetPosition(Vec3 pos_);
	void SetVelocity(Vec3 vel_);
	void SetOrientation(Quaternion orientation_);
	void SetRotation(float degrees);

	//void setDescriptorSet(const DescriptorSetInfo descriptorSetInfo_) { descriptorSetInfo = descriptorSetInfo_; }
	DescriptorSetInfo getDescriptorSetInfo() const { return descriptorSetInfo; }

	void Update(float deltaTime);
	void Render(VulkanRenderer* vRenderer, const PipelineInfo pipelineInfo);
};