#include "LiquidGlassPassElement.hpp"
#include "LiquidGlassDecoration.hpp"
#include "globals.hpp"

#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/render/OpenGL.hpp>

LiquidGlassPassElement::LiquidGlassPassElement(const SLiquidGlassData& data) : m_data(data) {}

void LiquidGlassPassElement::draw(const CRegion& damage) {
    m_data.deco->renderPass(g_pHyprOpenGL->m_renderData.pMonitor.lock(), m_data.a);
}

std::optional<CBox> LiquidGlassPassElement::boundingBox() {
    const auto PWINDOW = m_data.deco->getOwner();
    if (!PWINDOW)
        return std::nullopt;

    const auto PWINDOWWORKSPACE = PWINDOW->m_workspace;
    auto       surfaceBox       = PWINDOW->getWindowMainSurfaceBox();

    if (PWINDOWWORKSPACE && PWINDOWWORKSPACE->m_renderOffset->isBeingAnimated() && !PWINDOW->m_pinned)
        surfaceBox.translate(PWINDOWWORKSPACE->m_renderOffset->value());
    surfaceBox.translate(PWINDOW->m_floatingOffset);

    return surfaceBox;
}

bool LiquidGlassPassElement::needsLiveBlur() {
    return false;
}

bool LiquidGlassPassElement::needsPrecomputeBlur() {
    return false;
}
