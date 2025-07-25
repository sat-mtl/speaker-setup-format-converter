#pragma once
#include "speakerview_model.hpp"

#include <optional>

namespace spatparse::spatgris
{
std::optional<file> parse(std::string_view input);

std::vector<loudspeaker*> all_speakers(file& f);
std::vector<const loudspeaker*> all_speakers(const file& f);

struct fixup_options
{
  std::optional<double> normalize{};
  bool recenter{};
};

void fixup(file& f, fixup_options opts = {});
std::string to_string(const file& f);
std::string to_string_v4(const file& f);
}
