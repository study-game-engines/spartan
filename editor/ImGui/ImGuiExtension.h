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

#pragma once

//= INCLUDES ===============================
#include <string>
#include <variant>
#include <chrono>
#include "RHI/RHI_Texture.h"
#include "RHI/RHI_Texture2D.h"
#include "Rendering/Renderer.h"
#include "Resource/ResourceCache.h"
#include "Core/ThreadPool.h"
#include "Input/Input.h"
#include "World/World.h"
#include "World/Components/Camera.h"
#include "Display/Display.h"
#include "../WidgetsDeferred/IconProvider.h"
#include "Source/imgui_internal.h"
#include "../Editor.h"
#include "Rendering/Mesh.h"
//==========================================

class EditorHelper
{
public:
    static void Initialize(Editor* editor_)
    {
        editor = editor_;
    }

    static void LoadMesh(const std::string& file_path, const uint32_t load_flags)
    {
        // Load the model asynchronously
        Spartan::ThreadPool::AddTask([file_path, load_flags]()
        {
            Spartan::ResourceCache::Load<Spartan::Mesh>(file_path, load_flags);
        });
    }

    static void LoadWorld(const std::string& file_path)
    {
        // Loading a world resets everything so it's important to ensure that no tasks are running
        Spartan::ThreadPool::Flush(true);

        // Load the scene asynchronously
        Spartan::ThreadPool::AddTask([file_path]()
        {
            Spartan::World::LoadFromFile(file_path);
        });
    }

    static void SaveWorld(const std::string& file_path)
    {
        // Save the scene asynchronously
        Spartan::ThreadPool::AddTask([file_path]()
        {
            Spartan::World::SaveToFile(file_path);
        });
    }

    static Editor* editor;
};

namespace ImGui_SP
{
    enum class DragPayloadType
    {
        Texture,
        Entity,
        Model,
        Audio,
        Material,
        Undefined
    };

    enum class ButtonPress
    {
        Yes,
        No,
        Undefined
    };

    static const ImVec4 default_tint(1, 1, 1, 1);

    static float GetWindowContentRegionWidth()
    {
        return ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    }

    // Collapsing header
    static bool collapsing_header(const char* label, ImGuiTreeNodeFlags flags = 0)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        bool result = ImGui::CollapsingHeader(label, flags);
        ImGui::PopStyleVar();
        return result;
    }

    // Button
    static bool button(const char* label, const ImVec2& size = ImVec2(0, 0))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        bool result = ImGui::Button(label, size);
        ImGui::PopStyleVar();
        return result;
    }

    static bool button_centered_on_line(const char* label, float alignment = 0.5f)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        float size  = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
        float avail = ImGui::GetContentRegionAvail().x;

        float off = (avail - size) * alignment;
        if (off > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }

        return ImGui::Button(label);
    }

    static bool image_button(uint64_t id, Spartan::RHI_Texture* texture, const IconType icon, const Spartan::Math::Vector2& size, bool border)
    {
        if (!border)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        }

        // Get texture from icon enum (if provided)
        if (!texture && icon != IconType::Undefined)
        {
            texture = IconProvider::GetTextureByType(icon);
        }

        // Compute ID
        id += static_cast<uint64_t>(icon);
        id += texture ? reinterpret_cast<uint64_t>(&texture) : 0;

        bool result = ImGui::ImageButton
        (
            std::to_string(id).c_str(),        // str_id
            static_cast<ImTextureID>(texture), // user_texture_id
            size,                              // size
            ImVec2(0, 0),                      // uv0
            ImVec2(1, 1),                      // uv1
            ImColor(0, 0, 0, 0),               // bg_col
            default_tint                       // tint_col
        );

        if (!border)
        {
            ImGui::PopStyleVar();
        }

        return result;
    }

    static void image(const Thumbnail& thumbnail, const float size)
    {
        ImGui::Image(
            static_cast<ImTextureID>(IconProvider::GetTextureByThumbnail(thumbnail)),
            ImVec2(size, size),
            ImVec2(0, 0),
            ImVec2(1, 1),
            default_tint,       // tint
            ImColor(0, 0, 0, 0) // border
        );
    }

    static void image(Spartan::RHI_Texture* texture, const Spartan::Math::Vector2& size, bool border = false)
    {
        if (!border)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        }

        ImGui::Image(
            static_cast<ImTextureID>(texture),
            size,
            ImVec2(0, 0),
            ImVec2(1, 1),
            default_tint,       // tint
            ImColor(0, 0, 0, 0) // border
        );

        if (!border)
        {
            ImGui::PopStyleVar();
        }
    }

    static void image(Spartan::RHI_Texture* texture, const ImVec2& size, const ImVec4& tint = default_tint, const ImColor& border = ImColor(0, 0, 0, 0))
    {
        ImGui::Image(
            static_cast<ImTextureID>(texture),
            size,
            ImVec2(0, 0),
            ImVec2(1, 1),
            tint,
            border
        );
    }

    static void image(const IconType icon, const float size)
    {
        ImGui::Image(
            static_cast<void*>(IconProvider::GetTextureByType(icon)),
            ImVec2(size, size),
            ImVec2(0, 0),
            ImVec2(1, 1),
            default_tint,       // tint
            ImColor(0, 0, 0, 0) // border
        );
    }

    struct DragDropPayload
    {
        using DataVariant = std::variant<const char*, uint64_t>;
        DragDropPayload(const DragPayloadType type = DragPayloadType::Undefined, const DataVariant data = nullptr)
        {
            this->type = type;
            this->data = data;
        }
        DragPayloadType type;
        DataVariant data;
    };

    static void create_drag_drop_paylod(const DragDropPayload& payload)
    {
        ImGui::SetDragDropPayload(reinterpret_cast<const char*>(&payload.type), reinterpret_cast<const void*>(&payload), sizeof(payload), ImGuiCond_Once);
    }

    static DragDropPayload* receive_drag_drop_payload(DragPayloadType type)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const auto payload_imgui = ImGui::AcceptDragDropPayload(reinterpret_cast<const char*>(&type)))
            {
                return static_cast<DragDropPayload*>(payload_imgui->Data);
            }
            ImGui::EndDragDropTarget();
        }

        return nullptr;
    }

    // Image slot
    static void image_slot(const std::shared_ptr<Spartan::RHI_Texture>& texture_in, const std::function<void(const std::shared_ptr<Spartan::RHI_Texture>&)>& setter)
    {
        const ImVec2 slot_size  = ImVec2(80, 80);
        const float button_size = 15.0f;

        // Image
        ImGui::BeginGroup();
        {
            Spartan::RHI_Texture* texture = texture_in.get();
            const ImVec2 pos_image        = ImGui::GetCursorPos();
            const ImVec2 pos_button       = ImVec2(ImGui::GetCursorPosX() + slot_size.x - button_size * 2.0f + 6.0f, ImGui::GetCursorPosY() + 1.0f);

            uint32_t id = static_cast<uint32_t>(pos_button.x + pos_button.y);

            // Remove button
            if (texture != nullptr)
            {
                ImGui::SetCursorPos(pos_button);
                if (image_button(id, nullptr, IconType::Component_Material_RemoveTexture, button_size, true))
                {
                    texture = nullptr;
                    setter(nullptr);
                }
            }

            // Image
            ImVec4 colro_tint   = (texture != nullptr) ? ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 0);
            ImVec4 color_border = ImVec4(1, 1, 1, 0.5f);
            ImGui::SetCursorPos(pos_image);
            image(texture, slot_size, colro_tint, color_border);

            // Remove button - Does nothing, drawn again just to be visible
            if (texture != nullptr)
            {
                ImGui::SetCursorPos(pos_button);
                image_button(id, nullptr, IconType::Component_Material_RemoveTexture, button_size, true);
            }
        }
        ImGui::EndGroup();

        // Drop target
        if (auto payload = receive_drag_drop_payload(DragPayloadType::Texture))
        {
            try
            {
                if (const auto tex = Spartan::ResourceCache::Load<Spartan::RHI_Texture2D>(std::get<const char*>(payload->data)))
                {
                    setter(tex);
                }
            }
            catch (const std::bad_variant_access& e) { SP_LOG_ERROR("%s", e.what()); }
        }
    }

    static void tooltip(const char* text)
    {
        SP_ASSERT_MSG(text != nullptr, "Text is null");

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text(text);
            ImGui::EndTooltip();
        }
    }

    // A drag float which will wrap the mouse cursor around the edges of the screen
    static void draw_float_wrap(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const ImGuiSliderFlags flags = 0)
    {
        static const uint32_t screen_edge_padding = 10;
        static bool needs_to_wrap                 = false;

        // Wrap
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            Spartan::Math::Vector2 position_cursor = Spartan::Input::GetMousePosition();
            float position_left                    = static_cast<float>(screen_edge_padding);
            float position_right                   = static_cast<float>(Spartan::Display::GetWidth() - screen_edge_padding);
            bool is_on_right_screen_edge           = position_cursor.x >= position_right;
            bool is_on_left_screen_edge            = position_cursor.x <= position_left;

            if (is_on_right_screen_edge)
            {
                position_cursor.x = position_left + 1;
                needs_to_wrap     = true;
            }
            else if (is_on_left_screen_edge)
            {
                position_cursor.x = position_right - 1;
                needs_to_wrap     = true;
            }

            if (needs_to_wrap)
            {
                // Update mouse position
                ImGuiIO& imgui_io        = ImGui::GetIO();
                imgui_io.MousePos        = position_cursor;
                // Set an invalid previous position to eliminate big screen wrap delta (see ImGui::UpdateMouseInputs())
                imgui_io.MousePosPrev    = ImVec2(-FLT_MAX, -FLT_MAX); 
                imgui_io.WantSetMousePos = true;

                needs_to_wrap = false;
            }
        }

        ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags);
    }

    static bool combo_box(const char* label, const std::vector<std::string>& options, uint32_t* selection_index)
    {
        // Clamp the selection index in case it's larger than the actual option count.
        const uint32_t option_count = static_cast<uint32_t>(options.size());
        if (*selection_index >= option_count)
        {
            *selection_index = option_count - 1;
        }

        bool selection_made          = false;
        std::string selection_string = options[*selection_index];

        if (ImGui::BeginCombo(label, selection_string.c_str()))
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(options.size()); i++)
            {
                const bool is_selected = *selection_index == i;

                if (ImGui::Selectable(options[i].c_str(), is_selected))
                {
                    *selection_index    = i;
                    selection_made      = true;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        return selection_made;
    }

    static void vector3(const char* label, Spartan::Math::Vector3& vector)
    {
        const float label_indetation = 15.0f;

        const auto show_float = [](Spartan::Math::Vector3 axis, float* value)
        {
            const float label_float_spacing = 15.0f;
            const float step                = 0.01f;
            const std::string format        = "%.4f";

            // Label
            ImGui::TextUnformatted(axis.x == 1.0f ? "x" : axis.y == 1.0f ? "y" : "z");
            ImGui::SameLine(label_float_spacing);
            Spartan::Math::Vector2 pos_post_label = ImGui::GetCursorScreenPos();

            // Float
            ImGui::PushItemWidth(128.0f);
            ImGui::PushID(static_cast<int>(ImGui::GetCursorPosX() + ImGui::GetCursorPosY()));
            ImGui_SP::draw_float_wrap("##no_label", value, step, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), format.c_str());
            ImGui::PopID();
            ImGui::PopItemWidth();

            // Axis color
            static const ImU32 color_x                 = IM_COL32(168, 46, 2, 255);
            static const ImU32 color_y                 = IM_COL32(112, 162, 22, 255);
            static const ImU32 color_z                 = IM_COL32(51, 122, 210, 255);
            static const Spartan::Math::Vector2 size   = Spartan::Math::Vector2(4.0f, 19.0f);
            static const Spartan::Math::Vector2 offset = Spartan::Math::Vector2(5.0f, 4.0);
            pos_post_label += offset;
            ImRect axis_color_rect = ImRect(pos_post_label.x, pos_post_label.y, pos_post_label.x + size.x, pos_post_label.y + size.y);
            ImGui::GetWindowDrawList()->AddRectFilled(axis_color_rect.Min, axis_color_rect.Max, axis.x == 1.0f ? color_x : axis.y == 1.0f ? color_y : color_z);
        };

        ImGui::BeginGroup();
        ImGui::Indent(label_indetation);
        ImGui::TextUnformatted(label);
        ImGui::Unindent(label_indetation);
        show_float(Spartan::Math::Vector3(1.0f, 0.0f, 0.0f), &vector.x);
        show_float(Spartan::Math::Vector3(0.0f, 1.0f, 0.0f), &vector.y);
        show_float(Spartan::Math::Vector3(0.0f, 0.0f, 1.0f), &vector.z);
        ImGui::EndGroup();
    };

    inline ButtonPress window_yes_no(const char* title, const char* text)
    {
        // Set position
        ImVec2 position     = ImVec2(Spartan::Display::GetWidth() * 0.5f, Spartan::Display::GetHeight() * 0.5f);
        ImVec2 pivot_center = ImVec2(0.5f, 0.5f);
        ImGui::SetNextWindowPos(position, ImGuiCond_Always, pivot_center);

        // Window
        ButtonPress button_press = ButtonPress::Undefined;
        if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Text(text);

            if (ImGui_SP::button_centered_on_line("Yes", 0.4f))
            {
                button_press = ButtonPress::Yes;
            }

            ImGui::SameLine();

            if (ImGui::Button("No"))
            {
                button_press = ButtonPress::No;
            }
        }
        ImGui::End();

        return button_press;
    }
}
