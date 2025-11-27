#pragma once

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <hyprland/src/render/Framebuffer.hpp>

class LiquidGlassDecoration : public IHyprWindowDecoration {
  public:
    LiquidGlassDecoration(PHLWINDOW pWindow);
    virtual ~LiquidGlassDecoration() = default;

    virtual SDecorationPositioningInfo getPositioningInfo();
    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);
    virtual void                       draw(PHLMONITOR, float const& a);
    virtual eDecorationType            getDecorationType();
    virtual void                       updateWindow(PHLWINDOW);
    virtual void                       damageEntire();
    virtual eDecorationLayer           getDecorationLayer();
    virtual uint64_t                   getDecorationFlags();
    virtual std::string                getDisplayName();

    PHLWINDOW                          getOwner();
    void                               renderPass(PHLMONITOR pMonitor, const float& a);

    WP<LiquidGlassDecoration>          m_self;

  private:
    PHLWINDOWREF m_pWindow;
    CFramebuffer m_sampleFB;

    void         sampleBackground(CFramebuffer& sourceFB, CBox box);
    void         applyLiquidGlassShader(CFramebuffer& sourceFB, CFramebuffer& targetFB,
                                        CBox& rawBox, CBox& transformedBox, float radius);

    friend class LiquidGlassPassElement;
};
