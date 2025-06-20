#pragma once

#include "ease_model.hpp"

#include <optional>
namespace spatparse::ease
{
std::optional<file> parse(std::string_view input);
std::string to_string(const file& f);
}
