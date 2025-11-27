#include "LiquidGlassDecoration.hpp"
#include "globals.hpp"
#include "shaders.hpp"

#include <GLES3/gl32.h>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/Shader.hpp>
#include <hyprland/src/helpers/Color.hpp>

static std::string loadShader(const char* fileName) {
    if (shaders.contains(fileName)) {
        return shaders.at(fileName);
    }

    const std::string message = std::format("[liquid-glass] Couldn't load shader {}", fileName);
    HyprlandAPI::addNotification(PHANDLE, message, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
    throw std::runtime_error(message);
}

static void initShader() {
    const char* shaderFile = "liquidGlass.frag";
    GLuint      prog       = g_pHyprOpenGL->createProgram(
        g_pHyprOpenGL->m_shaders->TEXVERTSRC, loadShader(shaderFile), true);

    if (prog == 0) {
        const std::string message = std::format("[liquid-glass] Couldn't compile shader {}", shaderFile);
        HyprlandAPI::addNotification(PHANDLE, message, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error(message);
    }

    g_pGlobalState->shader.program                            = prog;
    g_pGlobalState->shader.uniformLocations[SHADER_PROJ]      = glGetUniformLocation(prog, "proj");
    g_pGlobalState->shader.uniformLocations[SHADER_POS_ATTRIB] = glGetAttribLocation(prog, "pos");
    g_pGlobalState->shader.uniformLocations[SHADER_TEX_ATTRIB] = glGetAttribLocation(prog, "texcoord");
    g_pGlobalState->shader.uniformLocations[SHADER_TEX]       = glGetUniformLocation(prog, "tex");
    g_pGlobalState->shader.uniformLocations[SHADER_RADIUS]    = glGetUniformLocation(prog, "radius");
    g_pGlobalState->shader.uniformLocations[SHADER_TOP_LEFT]  = glGetUniformLocation(prog, "topLeft");
    g_pGlobalState->shader.uniformLocations[SHADER_FULL_SIZE] = glGetUniformLocation(prog, "fullSize");

    g_pGlobalState->shader.createVao();
}

static void onNewWindow(void* self, std::any data) {
    const auto PWINDOW = std::any_cast<PHLWINDOW>(data);

    // Check if decoration already exists
    if (std::ranges::any_of(PWINDOW->m_windowDecorations,
                            [](const auto& d) { return d->getDisplayName() == "LiquidGlass"; }))
        return;

    auto deco = makeUnique<LiquidGlassDecoration>(PWINDOW);
    g_pGlobalState->decorations.emplace_back(deco);
    deco->m_self = deco;
    HyprlandAPI::addWindowDecoration(PHANDLE, PWINDOW, std::move(deco));
}

static void onCloseWindow(void* self, std::any data) {
    const auto PWINDOW = std::any_cast<PHLWINDOW>(data);

    std::erase_if(g_pGlobalState->decorations, [PWINDOW](const auto& deco) {
        auto locked = deco.lock();
        return !locked || locked->getOwner() == PWINDOW;
    });
}

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(
            PHANDLE,
            "[liquid-glass] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
            CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[liquid-glass] Version mismatch");
    }

    g_pGlobalState = std::make_unique<SGlobalState>();

    initShader();

    // Register callbacks
    static auto P1 = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "openWindow",
        [&](void* self, SCallbackInfo& info, std::any data) { onNewWindow(self, data); });

    static auto P2 = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "closeWindow",
        [&](void* self, SCallbackInfo& info, std::any data) { onCloseWindow(self, data); });

    // Register config values
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:liquid-glass:enabled", Hyprlang::INT{1});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:liquid-glass:refraction_strength", Hyprlang::FLOAT{0.08});

    // Add decoration to existing windows
    for (auto& w : g_pCompositor->m_windows) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        onNewWindow(nullptr, std::any(w));
    }

    HyprlandAPI::reloadConfig();

    return {"liquid-glass", "A plugin to add liquid glass effect to windows", "xiaoxigua-1", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    // Clean up decorations
    for (auto& deco : g_pGlobalState->decorations) {
        auto locked = deco.lock();
        if (locked) {
            auto owner = locked->getOwner();
            if (owner)
                owner->removeWindowDeco(locked.get());
        }
    }

    g_pHyprRenderer->m_renderPass.removeAllOfType("LiquidGlassPassElement");
    g_pGlobalState->shader.destroy();
    g_pGlobalState.reset();
}
