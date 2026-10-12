#import <Foundation/Foundation.h>

#include "sentype-settings.h"

static NSString* const kMethodKey = @"Method";        // "telex" | "vni"
static NSString* const kToneKey = @"TonePlacement";   // "modern" | "classic"

sentype::Config sentype_load_config() {
  NSUserDefaults* d = NSUserDefaults.standardUserDefaults;
  sentype::Config cfg;
  cfg.method = [[d stringForKey:kMethodKey] isEqualToString:@"vni"] ? sentype::Method::Vni
                                                                    : sentype::Method::Telex;
  cfg.tone_style = [[d stringForKey:kToneKey] isEqualToString:@"classic"]
                       ? sentype::ToneStyle::Classic
                       : sentype::ToneStyle::Modern;
  return cfg;
}

void sentype_save_method(sentype::Method method) {
  [NSUserDefaults.standardUserDefaults setObject:method == sentype::Method::Vni ? @"vni" : @"telex"
                                          forKey:kMethodKey];
}

void sentype_save_tone_style(sentype::ToneStyle style) {
  [NSUserDefaults.standardUserDefaults
      setObject:style == sentype::ToneStyle::Classic ? @"classic" : @"modern"
         forKey:kToneKey];
}
