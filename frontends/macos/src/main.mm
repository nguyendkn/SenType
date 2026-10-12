// Entry point: registers the IMK server named in Info.plist and runs the app loop.
#import <Cocoa/Cocoa.h>
#import <InputMethodKit/InputMethodKit.h>

static IMKServer* gServer;

int main(int, const char*[]) {
  @autoreleasepool {
    NSBundle* bundle = NSBundle.mainBundle;
    NSString* name = bundle.infoDictionary[@"InputMethodConnectionName"];
    if (!name) {
      NSLog(@"SenType: InputMethodConnectionName missing from Info.plist");
      return 1;
    }
    gServer = [[IMKServer alloc] initWithName:name bundleIdentifier:bundle.bundleIdentifier];
    [NSApplication sharedApplication];
    [NSApp run];
  }
  return 0;
}
