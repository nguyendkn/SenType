// Property checks shared by the libFuzzer target and the seeded random ctest.
// A byte string is decoded into key events (see fuzz-invariants.cpp); violations print a
// message and abort(), which libFuzzer reports as a crash and ctest as a failure.
#pragma once

#include <cstddef>
#include <cstdint>

namespace sentype_fuzz {

// Runs every invariant over one input. Never returns on violation.
void check_input(const uint8_t* data, size_t size);

}  // namespace sentype_fuzz
