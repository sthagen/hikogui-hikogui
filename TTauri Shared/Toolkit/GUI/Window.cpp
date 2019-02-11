//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Window.hpp"
#include "Device.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

void Window::buildSwapchain(void)
{
    surfaceCapabilities = device->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

    // Figure out the best way of sharing data between the present and graphic queues.
    vk::SharingMode sharingMode;
    uint32_t sharingQueueFamilyCount;
    uint32_t sharingQueueFamilyIndices[2] = {
        device->graphicQueue->queueFamilyIndex,
        device->presentQueue->queueFamilyIndex
    };
    uint32_t *sharingQueueFamilyIndicesPtr;

    if (device->presentQueue->queueCapabilities.handlesGraphicsAndPresent()) {
        sharingMode = vk::SharingMode::eExclusive;
        sharingQueueFamilyCount = 0;
        sharingQueueFamilyIndicesPtr = nullptr;
    } else {
        sharingMode = vk::SharingMode::eConcurrent;
        sharingQueueFamilyCount = 2;
        sharingQueueFamilyIndicesPtr = sharingQueueFamilyIndices;
    }

    auto imageCount = std::clamp(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

    uint32_t imageArrayLayers = 1;
    vk::Bool32 clipped = VK_TRUE;

    vk::Extent2D imageExtent = surfaceCapabilities.currentExtent;
    if (imageExtent.width == std::numeric_limits<uint32_t>::max() or imageExtent.height == std::numeric_limits<uint32_t>::max()) {
        imageExtent.width = std::clamp(displayRectangle.extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        imageExtent.height = std::clamp(displayRectangle.extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        intrinsic,
        imageCount,
        device->bestSurfaceFormat.format,
        device->bestSurfaceFormat.colorSpace,
        imageExtent,
        imageArrayLayers,
        vk::ImageUsageFlags(),
        sharingMode,
        sharingQueueFamilyCount,
        sharingQueueFamilyIndicesPtr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        device->bestSurfacePresentMode,
        clipped
    );

    LOG_INFO("Creating swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

    swapchain = device->intrinsic.createSwapchainKHR(swapchainCreateInfo);

}

void Window::teardownSwapchain(void)
{
}

void Window::buildSwapchainAndPipeline(void)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if (state == WindowState::LINKED_TO_DEVICE) {
        // XXX setup swap chain.
        buildSwapchain();
        state = WindowState::READY_TO_DRAW;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::teardownSwapchainAndPipeline(void)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if (state == WindowState::READY_TO_DRAW) {
        // XXX teardown swap chain.
        teardownSwapchain();
        state = WindowState::LINKED_TO_DEVICE;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::setDevice(Device *device) {
    if (device) {
        {
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            if (state == WindowState::NO_DEVICE) {
                this->device = device;
                state = WindowState::LINKED_TO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }

        buildSwapchainAndPipeline();

    } else {
        teardownSwapchainAndPipeline();

        {
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            if (state == WindowState::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = WindowState::NO_DEVICE;
                
            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }
    }
}

void Window::frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    if (stateMutex.try_lock_shared()) {
        if (state == WindowState::READY_TO_DRAW) {

        }
        stateMutex.unlock_shared();
    }
}

}}}
