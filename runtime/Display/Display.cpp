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

//= INCLUDES =======
#include "pch.h"
#include "Display.h"
#include "sdl/SDL.h"
#include "Window.h"
//==================

#if defined(_MSC_VER)
#include <dxgi.h>
#include <dxgi1_6.h>
#include <wrl.h>
#pragma comment(lib, "dxgi.lib")
#endif

//= NAMESPACES =====
using namespace std;
//==================

namespace Spartan
{
    namespace
    {
        static vector<DisplayMode> display_modes;
        static bool hdr;
        static float luminance_min;
        static float luminance_max;
    }

    static void get_hdr_capabilities(bool* hdr, float* luminance_min, float* luminance_max)
    {
        *hdr           = false;
        *luminance_min = 0.0f;
        *luminance_max = 0.0f;

        #if defined(_MSC_VER)
            // Create DXGI factory
            Microsoft::WRL::ComPtr<IDXGIFactory6> factory;
            if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
            {
                SP_LOG_ERROR("Failed to create DXGI factory");
                return;
            }

            // Enumerate and get the primary adapter (GPU)
            Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
            for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    continue;
                }

                break;
            }

            if (!adapter)
            {
                SP_LOG_ERROR("No DXGI adapter found");
                return;
            }

            // Find primary display by detecting which display is being intersected the most by the engine window
            Microsoft::WRL::ComPtr<IDXGIOutput> output_primary;
            {
                UINT i = 0;
                Microsoft::WRL::ComPtr<IDXGIOutput> output_current;
                float best_intersection_area = -1;
                RECT window_rect;
                GetWindowRect(static_cast<HWND>(Window::GetHandleWindows()), &window_rect);
                while (adapter->EnumOutputs(i, &output_current) != DXGI_ERROR_NOT_FOUND)
                {
                    // Get the rectangle bounds of the app window
                    int ax1 = window_rect.left;
                    int ay1 = window_rect.top;
                    int ax2 = window_rect.right;
                    int ay2 = window_rect.bottom;

                    // Get the rectangle bounds of current output
                    DXGI_OUTPUT_DESC desc;
                    if (FAILED(output_current->GetDesc(&desc)))
                    {
                        SP_LOG_ERROR("Failed to get output description");
                        return;
                    }

                    RECT r = desc.DesktopCoordinates;
                    int bx1 = r.left;
                    int by1 = r.top;
                    int bx2 = r.right;
                    int by2 = r.bottom;

                    // Compute the intersection
                    int intersectArea = max(0, min(ax2, bx2) - max(ax1, bx1)) * max(0, min(ay2, by2) - max(ay1, by1));
                    if (intersectArea > best_intersection_area)
                    {
                        output_primary = output_current;
                        best_intersection_area = static_cast<float>(intersectArea);
                    }

                    i++;
                }
            }

            // Get display capabilities
            Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(output_primary.As(&output6)))
            {
                DXGI_OUTPUT_DESC1 desc;
                if (SUCCEEDED(output6->GetDesc1(&desc)))
                {
                    *hdr = desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 || desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                    *luminance_min = desc.MinLuminance;
                    *luminance_max = desc.MaxLuminance;
                }
            }
        #else
            SP_LOG_ERROR("HDR support detection not implemented");
        #endif
    }

    void Display::RegisterDisplayMode(const uint32_t width, const uint32_t height, uint32_t hz, uint8_t display_index)
    {
        SP_ASSERT_MSG(width  != 0,    "width can't be zero");
        SP_ASSERT_MSG(height != 0,    "height can't be zero");
        SP_ASSERT_MSG(hz     != 0.0f, "hz can't be zero");

        // Early exit if the display mode is already registered
        for (const DisplayMode& display_mode : display_modes)
        {
            if (display_mode.width         == width  &&
                display_mode.height        == height &&
                display_mode.hz            == hz     &&
                display_mode.display_index == display_index)
                return;
        }

        // Add the new display mode
        display_modes.emplace_back(width, height, hz, display_index);

        // Sort display modes based on width, descending order
        sort(display_modes.begin(), display_modes.end(), [](const DisplayMode& display_mode_a, const DisplayMode& display_mode_b)
        {
            return display_mode_a.width > display_mode_b.width;
        });

        // Set the FPS limit to the HZ corresponding to our optimal display mode
        if (GetRefreshRate() > Timer::GetFpsLimit())
        {
            Timer::SetFpsLimit(static_cast<float>(GetRefreshRate()));
        }
    }

    void Display::DetectDisplayModes()
    {
        display_modes.clear();

        // Get display index of the display that contains this window
        int display_index = SDL_GetWindowDisplayIndex(static_cast<SDL_Window*>(Window::GetHandleSDL()));
        if (display_index < 0)
        {
            SP_LOG_ERROR("Failed to window display index");
            return;
        }

        // Get display mode count
        int display_mode_count = SDL_GetNumDisplayModes(display_index);
        if (display_mode_count <= 0)
        {
            SP_LOG_ERROR("Failed to get display mode count");
            return;
        }

        // Register display modes
        for (int display_mode_index = 0; display_mode_index < display_mode_count; display_mode_index++)
        {
            SDL_DisplayMode display_mode;
            if (SDL_GetDisplayMode(display_index, display_mode_index, &display_mode) == 0)
            {
                RegisterDisplayMode(display_mode.w, display_mode.h, display_mode.refresh_rate, display_index);
            }
            else
            {
                SP_LOG_ERROR("Failed to get display mode %d for display %d", display_mode_index, display_index);
            }
        }

        // Detect HDR capabilities
        get_hdr_capabilities(&hdr, &luminance_min, &luminance_max);
        SP_LOG_INFO("HDR: %s, Luminance: %f, %f", hdr ? "true" : "false", luminance_min, luminance_max);
    }

    const vector<DisplayMode>& Display::GetDisplayModes()
    {
        return display_modes;
    }

    uint32_t Display::GetWidth()
    {
        SDL_DisplayMode display_mode;
        SP_ASSERT(SDL_GetCurrentDisplayMode(GetIndex(), &display_mode) == 0);

        return display_mode.w;
    }

    uint32_t Display::GetHeight()
    {
        int display_index = SDL_GetWindowDisplayIndex(static_cast<SDL_Window*>(Window::GetHandleSDL()));

        SDL_DisplayMode display_mode;
        SP_ASSERT(SDL_GetCurrentDisplayMode(GetIndex(), &display_mode) == 0);

        return display_mode.h;
    }

    uint32_t Display::GetRefreshRate()
    {
        SDL_DisplayMode display_mode;
        SP_ASSERT(SDL_GetCurrentDisplayMode(GetIndex(), &display_mode) == 0);

        return display_mode.refresh_rate;
    }

    uint32_t Display::GetIndex()
    {
        int index = SDL_GetWindowDisplayIndex(static_cast<SDL_Window*>(Window::GetHandleSDL()));

        // during engine startup, the window doesn't exist yet, therefore it's not displayed by any monitor.
        // in this case the index can be -1, so we'll instead set the index to 0 (whatever the primary display is)
        return index != -1 ? index : 0;
    }

    bool Display::GetHdr()
    {
        return hdr;
    }
}
