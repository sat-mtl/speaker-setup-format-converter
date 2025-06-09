#pragma once
#include "csv_model.hpp"

#include <optional>
#include <string_view>

namespace spatparse::csv
{
std::optional<file> parse(std::string_view bytes);
}
