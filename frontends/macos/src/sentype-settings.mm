#import <Foundation/Foundation.h>

#include "sentype-settings.h"
#include "sentype/settings.hpp"

static NSString* const kMethodKey = @"Method";        // "telex" | "vni"
static NSString* const kToneKey = @"TonePlacement";   // "modern" | "classic"

sentype::Config sentype_load_config() {
  NSUserDefaults* d = NSUserDefaults.standardUserDefaults;
  sentype::Config cfg;
  cfg.method = sentype::parse_method([[d stringForKey:kMethodKey] UTF8String] ?: "");
  cfg.tone_style = sentype::parse_tone_style([[d stringForKey:kToneKey] UTF8String] ?: "");
  return cfg;
}

void sentype_save_method(sentype::Method method) {
  [NSUserDefaults.standardUserDefaults setObject:@(sentype::to_string(method)) forKey:kMethodKey];
}

void sentype_save_tone_style(sentype::ToneStyle style) {
  [NSUserDefaults.standardUserDefaults setObject:@(sentype::to_string(style)) forKey:kToneKey];
}
