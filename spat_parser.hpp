#pragma once
#include "spat_model.hpp"

#include <optional>

namespace spatparse::spat
{
std::optional<file> parse(std::string_view input);
std::string to_string(const file& f);
}
