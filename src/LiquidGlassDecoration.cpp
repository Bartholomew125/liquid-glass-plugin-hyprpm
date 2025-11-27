#include "LiquidGlassDecoration.hpp"
#include "LiquidGlassPassElement.hpp"
#include "globals.hpp"

#include <GLES3/gl32.h>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprutils/math/Misc.hpp>
#include <hyprutils/math/Region.hpp>
#include <hyprutils/math/Vector2D.hpp>

LiquidGlassDecoration::LiquidGlassDecoration(PHLWINDOW pWindow)
    : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
    pWindow->m_windowData.noBlur = true;
}

eDecorationLayer LiquidGlassDecoration::getDecorationLayer() {
    return DECORATION_LAYER_BOTTOM;
}

uint64_t LiquidGlassDecoration::getDecorationFlags() {
    return DECORATION_NON_SOLID;
}

eDecorationType LiquidGlassDecoration::getDecorationType() {
    return eDecorationType::DECORATION_CUSTOM;
}

std::string LiquidGlassDecoration::getDisplayName() {
    return "LiquidGlass";
}

SDecorationPositioningInfo LiquidGlassDecoration::getPositioningInfo() {
    SDecorationPositioningInfo info;
    info.priority       = 10000;
    info.policy         = DECORATION_POSITION_ABSOLUTE;
    info.desiredExtents = {{0, 0}, {0, 0}};
    return info;
}

void LiquidGlassDecoration::onPositioningReply(const SDecorationPositioningReply& reply) {
    // No-op
}

PHLWINDOW LiquidGlassDecoration::getOwner() {
    return m_pWindow.lock();
}

void LiquidGlassDecoration::draw(PHLMONITOR pMonitor, float const& a) {
    LiquidGlassPassElement::SLiquidGlassData data{this, a};
    g_pHyprRenderer->m_renderPass.add(makeUnique<LiquidGlassPassElement>(data));
}

void LiquidGlassDecoration::applyLiquidGlassShader(CFramebuffer& sourceFB, CFramebuffer& targetFB,
                                                    CBox& rawBox, CBox& transformedBox, float radius) {
    const auto TR = wlTransformToHyprutils(
        invertTransform(g_pHyprOpenGL->m_renderData.pMonitor->m_transform));

    Mat3x3 matrix = g_pHyprOpenGL->m_renderData.monitorProjection.projectBox(rawBox, TR, rawBox.rot);
    Mat3x3 glMatrix = g_pHyprOpenGL->m_renderData.projection.copy().multiply(matrix);
    auto   tex = sourceFB.getTexture();

    glMatrix.transpose();
    glBindFramebuffer(GL_FRAMEBUFFER, targetFB.getFBID());
    glActiveTexture(GL_TEXTURE0);
    tex->bind();
    g_pHyprOpenGL->useProgram(g_pGlobalState->shader.program);

    g_pGlobalState->shader.setUniformMatrix3fv(SHADER_PROJ, 1, GL_FALSE, glMatrix.getMatrix());
    g_pGlobalState->shader.setUniformInt(SHADER_TEX, 0);

    const auto TOPLEFT  = Vector2D(transformedBox.x, transformedBox.y);
    const auto FULLSIZE = Vector2D(transformedBox.width, transformedBox.height);
    const auto FBOSIZE  = Vector2D(targetFB.m_size.x, targetFB.m_size.y);

    g_pGlobalState->shader.setUniformFloat2(SHADER_TOP_LEFT, staticCast<float>(TOPLEFT.x), staticCast<float>(TOPLEFT.y));
    g_pGlobalState->shader.setUniformFloat2(SHADER_FULL_SIZE, staticCast<float>(FULLSIZE.x), staticCast<float>(FULLSIZE.y));
    glUniform2f(glGetUniformLocation(g_pGlobalState->shader.program, "fboSize"),
                staticCast<float>(FBOSIZE.x), staticCast<float>(FBOSIZE.y));

    g_pGlobalState->shader.setUniformFloat(SHADER_RADIUS, radius);

    glBindVertexArray(g_pGlobalState->shader.uniformLocations[SHADER_SHADER_VAO]);
    g_pHyprOpenGL->scissor(rawBox);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    g_pHyprOpenGL->scissor(nullptr);
}

void LiquidGlassDecoration::sampleBackground(CFramebuffer& sourceFB, CBox box) {
    if (m_sampleFB.m_size.x != box.width || m_sampleFB.m_size.y != box.height) {
        m_sampleFB.alloc(box.width, box.height, sourceFB.m_drmFormat);
    }

    int x0 = box.x;
    int x1 = box.x + box.width;
    int y0 = box.y;
    int y1 = box.y + box.height;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFB.getFBID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_sampleFB.getFBID());
    glBlitFramebuffer(x0, y0, x1, y1, 0, 0, box.width, box.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void LiquidGlassDecoration::renderPass(PHLMONITOR pMonitor, const float& a) {
    const auto PWINDOW    = m_pWindow.lock();
    if (!PWINDOW)
        return;

    const auto PWORKSPACE = PWINDOW->m_workspace;
    const auto WORKSPACEOFFSET =
        PWORKSPACE && !PWINDOW->m_pinned ? PWORKSPACE->m_renderOffset->value() : Vector2D();
    const auto SOURCE = g_pHyprOpenGL->m_renderData.currentFB;
    const auto RADIUS = PWINDOW->rounding();

    auto thisbox = PWINDOW->getWindowMainSurfaceBox();

    CBox wlrbox = thisbox.translate(WORKSPACEOFFSET)
                      .translate(-pMonitor->m_position + PWINDOW->m_floatingOffset)
                      .scale(pMonitor->m_scale)
                      .round();
    CBox transformBox = wlrbox;

    const auto TR = wlTransformToHyprutils(
        invertTransform(g_pHyprOpenGL->m_renderData.pMonitor->m_transform));
    transformBox.transform(TR, g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.x,
                           g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.y);

    sampleBackground(*SOURCE, transformBox);
    applyLiquidGlassShader(m_sampleFB, *SOURCE, wlrbox, transformBox, RADIUS);
}

void LiquidGlassDecoration::updateWindow(PHLWINDOW pWindow) {
    g_pDecorationPositioner->repositionDeco(this);
}

void LiquidGlassDecoration::damageEntire() {
    const auto PWINDOW = m_pWindow.lock();
    if (!PWINDOW)
        return;

    const auto PWINDOWWORKSPACE = PWINDOW->m_workspace;
    auto       surfaceBox       = PWINDOW->getWindowMainSurfaceBox();

    if (PWINDOWWORKSPACE && PWINDOWWORKSPACE->m_renderOffset->isBeingAnimated() && !PWINDOW->m_pinned)
        surfaceBox.translate(PWINDOWWORKSPACE->m_renderOffset->value());
    surfaceBox.translate(PWINDOW->m_floatingOffset);

    g_pHyprRenderer->damageBox(surfaceBox);
}
