#pragma once
#include "speakerview_model.hpp"

#include <optional>

namespace spatparse::speakerview
{
std::optional<file> parse(std::string_view input);

struct fixup_options
{
  std::optional<double> normalize{};
  bool recenter{};
};

void fixup(file& f, fixup_options opts = {});
std::string to_string(const file& f);
}
