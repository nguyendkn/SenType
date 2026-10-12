#pragma once
#include "sentype/engine.hpp"

// Reads HKCU\Software\SenType: Method ("Telex"|"VNI"), TonePlacement ("Modern"|"Classic").
sentype::Config load_config();

// Persists the typing method to HKCU\Software\SenType\Method (read back by load_config()).
void save_method(sentype::Method method);
