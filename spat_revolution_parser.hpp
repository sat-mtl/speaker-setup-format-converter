#pragma once
#include "spat_revolution_model.hpp"

#include <optional>
#include <string_view>

namespace spatparse::spat_revolution
{
std::optional<file> parse(std::string_view bytes);
std::string to_string(const file& f);
}