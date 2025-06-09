#pragma once
#include "aiira_model.hpp"

#include <optional>
#include <string_view>

namespace spatparse::aiira
{
std::optional<file> parse(std::string_view bytes);
}
