// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineFlat.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Device.hpp"

namespace TTauri::GUI::PipelineFlat {

using namespace TTauri;
using namespace std;
using namespace gsl;

PipelineFlat::PipelineFlat(Window const &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore PipelineFlat::render(vk::Framebuffer frameBuffer, vk::Semaphore inputSemaphore)
{
    device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (Vertex));

    return Pipeline_vulkan::render(frameBuffer, inputSemaphore);
}

void PipelineFlat::drawInCommandBuffer()
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    BOOST_ASSERT(tmpVertexBuffers.size() == tmpOffsets.size());

    device().flatPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { extent.width , extent.height };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

    let numberOfRectangles = vertexBufferData.size() / 4;
    let numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineFlat::createShaderStages() const {
    return device().flatPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineFlat::createDescriptorSetLayoutBindings() const {
    return { };
}

vector<vk::WriteDescriptorSet> PipelineFlat::createWriteDescriptorSet() const
{
    return { };
}

ssize_t PipelineFlat::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> PipelineFlat::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}

vk::VertexInputBindingDescription PipelineFlat::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineFlat::createVertexInputAttributeDescriptions() const {
    return Vertex::inputAttributeDescriptions();
}

void PipelineFlat::buildVertexBuffers()
{
    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof (Vertex) * PipelineFlat::maximumNumberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = device().mapMemory<Vertex>(vertexBufferAllocation);
}

void PipelineFlat::teardownVertexBuffers()
{
    device().unmapMemory(vertexBufferAllocation);
    device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
