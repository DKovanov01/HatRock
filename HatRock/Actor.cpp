#include "Actor.h"

void Actor::SetMesh(IndexedVertexBuffer mesh_) {
    mesh = mesh_;
}

void Actor::SetTexture(Sampler2D texture_) {
    texture = texture_;
}

void Actor::SetPosition(Vec3 pos_) {
    pos = pos_;
}

void Actor::SetVelocity(Vec3 vel_) {
    vel = vel_;
}

void Actor::SetOrientation(Quaternion orientation_) {
    orientation = orientation_;
}

void Actor::SetRotation(float degrees) {
    rotationMatrix = MMath::rotate(degrees, Vec3(0.0f, 1.0f, 0.0f));
}

bool Actor::OnCreate(VulkanRenderer* vRenderer) {
    DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice());

    //actorData = vRenderer->CreateUniformBuffers<ActorData>();
    descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &texture); // &texture replaced actorData | VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER replaced by VK_SHADER_STAGE_VERTEX_BIT replaced by 

    descriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains());

    return true;
}

void Actor::Update(float deltaTime) {
    pos += vel * deltaTime;
    vel += accel * deltaTime;

    modelMatrix = MMath::translate(pos) * rotationMatrix;
}

void Actor::Render(VulkanRenderer* vRenderer, const PipelineInfo pipelineInfo) {
    vRenderer->BindMesh(mesh);

    vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, descriptorSetInfo.descriptorSet, 1);

    vRenderer->SetPushConstant(pipelineInfo, modelMatrix);

    vRenderer->DrawIndexed(mesh);
}

void Actor::OnDestroy(VulkanRenderer* vRenderer) {

    vRenderer->DestroyDescriptorSet(descriptorSetInfo);
    vRenderer->DestroySampler2D(texture);
    vRenderer->DestroyIndexedMesh(mesh);
}