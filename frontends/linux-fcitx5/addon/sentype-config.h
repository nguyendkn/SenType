// User-facing options of the SenType addon (shown in fcitx5-configtool).
#pragma once

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-utils/i18n.h>

namespace sentype_fcitx {

enum class TypingMethod { Telex, VNI };
FCITX_CONFIG_ENUM_NAME_WITH_I18N(TypingMethod, N_("Telex"), N_("VNI"));

enum class TonePlacement { Modern, Classic };
FCITX_CONFIG_ENUM_NAME_WITH_I18N(TonePlacement, N_("Modern (hoà, thuý)"),
                                 N_("Classic (hòa, thúy)"));

FCITX_CONFIGURATION(
    SenTypeConfig,
    fcitx::OptionWithAnnotation<TypingMethod, TypingMethodI18NAnnotation> method{
        this, "Method", _("Typing method"), TypingMethod::Telex};
    fcitx::OptionWithAnnotation<TonePlacement, TonePlacementI18NAnnotation> tonePlacement{
        this, "TonePlacement", _("Tone mark placement"), TonePlacement::Modern};
    fcitx::Option<bool> spellCheck{this, "SpellCheck",
                                   _("Restore typed keys for non-Vietnamese words"), true};);

}  // namespace sentype_fcitx
