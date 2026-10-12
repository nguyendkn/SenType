// User settings persisted in NSUserDefaults, mapped to the core Config.
#pragma once

#include "sentype/engine.hpp"

sentype::Config sentype_load_config();
void sentype_save_method(sentype::Method method);
void sentype_save_tone_style(sentype::ToneStyle style);
