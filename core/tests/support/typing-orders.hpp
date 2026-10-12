// Generates the key sequences a person could type to produce a given syllable.
#pragma once

#include <string>

#include "vi-units.hpp"

namespace sentype_test {

enum class Order {
  ToneAtEnd,     // marks inline, tone after the last letter:    "nguwowif"
  ToneAfterNuc,  // marks inline, tone right after the nucleus:  "nguwowfi"
  FreeMarks,     // plain letters, then marks, then the tone:    "nguoiwf"
  FreeToneFirst  // plain letters, tone after the nucleus, then marks: "nguofiw"
};
constexpr Order kAllOrders[] = {Order::ToneAtEnd, Order::ToneAfterNuc, Order::FreeMarks,
                                Order::FreeToneFirst};

const char* order_name(Order o);
// Both return "" when the order does not apply (e.g. nothing to mark late, no tone).
std::string telex_keys(const Units& u, Order o);
std::string vni_keys(const Units& u, Order o);

}  // namespace sentype_test
