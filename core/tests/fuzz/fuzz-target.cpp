// libFuzzer entry point (clang -fsanitize=fuzzer,address,undefined).
#include "fuzz-invariants.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  sentype_fuzz::check_input(data, size);
  return 0;
}
