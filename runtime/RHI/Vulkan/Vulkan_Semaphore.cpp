/*
Copyright(c) 2016-2023 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//= INCLUDES =====================
#include "pch.h"
#include "../RHI_Semaphore.h"
#include "../RHI_Implementation.h"
#include "../Rendering/Renderer.h"
//================================

namespace Spartan
{
    static void create_semaphore(VkDevice device, const bool is_timeline, void*& resource)
    {
        SP_ASSERT(resource == nullptr);

        VkSemaphoreTypeCreateInfo semaphore_type_create_info = {};
        semaphore_type_create_info.sType                     = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphore_type_create_info.pNext                     = nullptr;
        semaphore_type_create_info.semaphoreType             = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue              = 0;

        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext                 = is_timeline ? &semaphore_type_create_info : nullptr;
        semaphore_create_info.flags                 = 0;

        // Create
        SP_ASSERT_MSG(
            vkCreateSemaphore(device, &semaphore_create_info, nullptr, reinterpret_cast<VkSemaphore*>(&resource)) == VK_SUCCESS,
            "Failed to create semaphore"
        );
    }

    static void destroy_semaphore(VkDevice device, void*& resource)
    {
        if (!resource)
            return;

        vkDestroySemaphore(device, static_cast<VkSemaphore>(resource), nullptr);
        resource = nullptr;
    }

    RHI_Semaphore::RHI_Semaphore(bool is_timeline /*= false*/, const char* name /*= nullptr*/)
    {
        m_is_timeline = is_timeline;

        create_semaphore(Renderer::GetRhiDevice()->GetRhiContext()->device, m_is_timeline, m_resource);

        // Name
        if (name)
        {
            m_name = name;
            vulkan_utility::debug::set_object_name(static_cast<VkSemaphore>(m_resource), name);
        }
    }

    RHI_Semaphore::~RHI_Semaphore()
    {
        if (!m_resource)
            return;

        // Wait in case it's still in use by the GPU
        Renderer::GetRhiDevice()->QueueWaitAll();

        destroy_semaphore(Renderer::GetRhiDevice()->GetRhiContext()->device, m_resource);
    }

    void RHI_Semaphore::Reset()
    {
        // Wait in case it's still in use by the GPU
        Renderer::GetRhiDevice()->QueueWaitAll();

        destroy_semaphore(Renderer::GetRhiDevice()->GetRhiContext()->device, m_resource);
        create_semaphore(Renderer::GetRhiDevice()->GetRhiContext()->device, m_is_timeline, m_resource);
        m_cpu_state = RHI_Sync_State::Idle;
    }

    void RHI_Semaphore::Wait(const uint64_t value, uint64_t timeout /*= std::numeric_limits<uint64_t>::max()*/)
    {
        SP_ASSERT(m_is_timeline);

        VkSemaphoreWaitInfo semaphore_wait_info = {};
        semaphore_wait_info.sType               = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        semaphore_wait_info.pNext               = nullptr;
        semaphore_wait_info.flags               = 0;
        semaphore_wait_info.semaphoreCount      = 1;
        semaphore_wait_info.pSemaphores         = reinterpret_cast<VkSemaphore*>(&m_resource);
        semaphore_wait_info.pValues             = &value;

        SP_ASSERT_MSG(vkWaitSemaphores(Renderer::GetRhiDevice()->GetRhiContext()->device, &semaphore_wait_info, timeout) == VK_SUCCESS,
            "Failed to wait for semaphore"
        );
    }

    void RHI_Semaphore::Signal(const uint64_t value)
    {
        SP_ASSERT(m_is_timeline);

        VkSemaphoreSignalInfo semaphore_signal_info = {};
        semaphore_signal_info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        semaphore_signal_info.pNext                 = nullptr;
        semaphore_signal_info.semaphore             = static_cast<VkSemaphore>(m_resource);
        semaphore_signal_info.value                 = value;

        SP_ASSERT_MSG(vkSignalSemaphore(Renderer::GetRhiDevice()->GetRhiContext()->device, &semaphore_signal_info) == VK_SUCCESS,
            "Failed to signal semaphore"
        );
    }

    uint64_t RHI_Semaphore::GetValue()
    {
        SP_ASSERT(m_is_timeline);

        uint64_t value = 0;
        SP_ASSERT_MSG(
            vkGetSemaphoreCounterValue(Renderer::GetRhiDevice()->GetRhiContext()->device, static_cast<VkSemaphore>(m_resource), &value) == VK_SUCCESS,
            "Failed to get semaphore counter value"
        );

        return value;
    }
}
