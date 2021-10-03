#pragma once
#include <vector>
#include <string>
#include "imgui.h"

namespace ImGui
{
    inline static auto vector_getter = [](void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    };

    inline bool Combo(const char* label, int* currIndex, const std::vector<std::string>& values)
    {
        if (values.empty()) { return false; }
        return Combo(label, currIndex, vector_getter,
            (void*)(&values), values.size());
    }
}