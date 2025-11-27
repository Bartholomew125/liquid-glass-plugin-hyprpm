#pragma once

#include <hyprland/src/render/pass/PassElement.hpp>
#include <hyprutils/math/Box.hpp>
#include <hyprutils/math/Region.hpp>

class LiquidGlassDecoration;

class LiquidGlassPassElement : public IPassElement {
  public:
    struct SLiquidGlassData {
        LiquidGlassDecoration* deco = nullptr;
        float                  a    = 1.F;
    };

    LiquidGlassPassElement(const SLiquidGlassData& data);
    virtual ~LiquidGlassPassElement() = default;

    virtual void                draw(const CRegion& damage);
    virtual bool                needsLiveBlur();
    virtual bool                needsPrecomputeBlur();
    virtual std::optional<CBox> boundingBox();

    virtual const char*         passName() {
        return "LiquidGlassPassElement";
    }

  private:
    SLiquidGlassData m_data;
};
