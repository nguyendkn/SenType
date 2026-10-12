#pragma once
#include "sentype/engine.hpp"

// Reads HKCU\Software\SenType: Method ("Telex"|"VNI"), TonePlacement ("Modern"|"Classic").
sentype::Config load_config();
