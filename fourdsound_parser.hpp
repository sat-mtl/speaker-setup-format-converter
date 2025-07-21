#pragma once
#include "fourdsound_model.hpp"

#include <optional>

namespace spatparse::fourdsound
{
std::optional<file> parse(std::string_view input);
std::string to_string(const file& f);
}