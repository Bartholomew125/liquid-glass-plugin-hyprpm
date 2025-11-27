#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Shader.hpp>
#include <memory>
#include <vector>

class LiquidGlassDecoration;

struct SGlobalState {
    std::vector<WP<LiquidGlassDecoration>> decorations;
    SShader                                shader;
};

inline HANDLE                        PHANDLE = nullptr;
inline std::unique_ptr<SGlobalState> g_pGlobalState;

template <typename T>
inline T staticCast(auto value) {
    return static_cast<T>(value);
}
