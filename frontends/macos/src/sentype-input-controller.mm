#import "sentype-input-controller.h"

#include <memory>

#include "sentype-settings.h"
#include "sentype/session.hpp"
#include "key-translation.h"

static const NSRange kNoReplacement = {NSNotFound, NSNotFound};

@implementation SenTypeInputController {
  std::unique_ptr<sentype::Session> _session;
  BOOL _hasMarked;  // marked text currently shown in the client
}

- (instancetype)initWithServer:(IMKServer*)server delegate:(id)delegate client:(id)inputClient {
  if ((self = [super initWithServer:server delegate:delegate client:inputClient])) {
    _session = std::make_unique<sentype::Session>(sentype_load_config());
  }
  return self;
}

- (NSUInteger)recognizedEvents:(id)sender {
  return NSEventMaskKeyDown;
}

// Applies a core result to the client: commit text first, then composition (preedit only).
- (void)apply:(const sentype::KeyResult&)r to:(id<IMKTextInput>)client {
  if (!r.commit.empty()) {
    [client insertText:@(r.commit.c_str()) replacementRange:kNoReplacement];
    _hasMarked = NO;  // insertText replaces the marked range
  }
  if (!r.preedit.empty()) {
    NSString* text = @(r.preedit.c_str());
    NSAttributedString* marked = [[NSAttributedString alloc]
        initWithString:text
            attributes:@{NSUnderlineStyleAttributeName : @(NSUnderlineStyleSingle)}];
    [client setMarkedText:marked
            selectionRange:NSMakeRange(text.length, 0)
          replacementRange:kNoReplacement];
    _hasMarked = YES;
  } else if (_hasMarked) {
    [client setMarkedText:@"" selectionRange:NSMakeRange(0, 0) replacementRange:kNoReplacement];
    _hasMarked = NO;
  }
}

- (BOOL)handleEvent:(NSEvent*)event client:(id)sender {
  if (event.type != NSEventTypeKeyDown) return NO;
  NSString* chars = event.characters ?: @"";
  std::u16string utf16(chars.length, u'\0');
  [chars getCharacters:reinterpret_cast<unichar*>(utf16.data())];
  const sentype::Key key = sentype_mac::translate_key(utf16.data(), utf16.size(), event.keyCode,
                                                      event.modifierFlags);
  const sentype::KeyResult r = _session->on_key(key);
  [self apply:r to:sender];
  return r.consumed;
}

// Called by the system/app when the composition must end (click elsewhere, etc).
- (void)commitComposition:(id)sender {
  [self apply:_session->flush() to:sender];
}

- (void)activateServer:(id)sender {
  _session->set_config(sentype_load_config());  // picks up menu changes
  [super activateServer:sender];
}

- (void)deactivateServer:(id)sender {
  [self commitComposition:sender];
  [super deactivateServer:sender];
}

#pragma mark Input-source menu

- (NSMenu*)menu {
  const sentype::Config cfg = sentype_load_config();
  NSMenu* menu = [[NSMenu alloc] initWithTitle:@"SenType"];
  NSMenuItem* telex = [menu addItemWithTitle:@"Telex" action:@selector(useTelex:) keyEquivalent:@""];
  NSMenuItem* vni = [menu addItemWithTitle:@"VNI" action:@selector(useVni:) keyEquivalent:@""];
  NSMenuItem* classic = [menu addItemWithTitle:@"Classic tone placement (hòa, thúy)"
                                        action:@selector(toggleClassic:)
                                 keyEquivalent:@""];
  telex.state = cfg.method == sentype::Method::Telex ? NSControlStateValueOn : NSControlStateValueOff;
  vni.state = cfg.method == sentype::Method::Vni ? NSControlStateValueOn : NSControlStateValueOff;
  classic.state = cfg.tone_style == sentype::ToneStyle::Classic ? NSControlStateValueOn
                                                                : NSControlStateValueOff;
  return menu;
}

- (void)useTelex:(id)sender { [self setMethod:sentype::Method::Telex client:self.client]; }
- (void)useVni:(id)sender { [self setMethod:sentype::Method::Vni client:self.client]; }

- (void)setMethod:(sentype::Method)method client:(id)client {
  [self commitComposition:client];
  sentype_save_method(method);
  _session->set_config(sentype_load_config());
}

- (void)toggleClassic:(id)sender {
  [self commitComposition:self.client];
  const bool classic = sentype_load_config().tone_style == sentype::ToneStyle::Classic;
  sentype_save_tone_style(classic ? sentype::ToneStyle::Modern : sentype::ToneStyle::Classic);
  _session->set_config(sentype_load_config());
}

@end
