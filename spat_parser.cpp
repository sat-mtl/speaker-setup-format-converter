#include "spat_parser.hpp"

#include <ctre.hpp>

#include <ranges>

namespace spatparse::spat
{
std::optional<file> parse(std::string_view input)
try
{
  file res;

  auto begin = input.find("/speaker/");
  if(begin == std::string_view::npos)
    return std::nullopt;

  input = input.substr(begin);

  constexpr char delimiter = '\n';
  std::ranges::lazy_split_view outer_view{input, delimiter};

  auto at_i = [&](int i) -> decltype(auto) {
    auto& vec = res.loudspeakers;
    if(vec.size() < i + 1)
    {
      vec.resize(i + 1);
    }
    return vec[i];
  };

  for(const auto word : std::views::split(input, '\n'))
  {
    std::string_view line{word};
    if(line.ends_with('\\') || line.ends_with('}'))
      line = line.substr(0, line.size() - 1);

    if(auto m = ctre::match<"/speaker/number (.*)">(line))
    {
      const int speaker_count = m.get<1>().to_number();
      res.loudspeakers.reserve(speaker_count);
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/delay (.*)">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      const auto delay = m.get<2>().to_number<double>();

      at_i(speaker_idx).delay = delay;
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/gain/db (.*)">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      const auto gain_db = m.get<2>().to_number<double>();

      at_i(speaker_idx).gain_db = gain_db;
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/name \"(.*)\"">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      const auto name = m.get<2>().to_view();

      at_i(speaker_idx).name = name;
    }
    else if(auto m = ctre::match<"/speakers/correction/delay (.*)">(line))
    {
      if(m == "\"auto\"")
      {
        res.header.correction.delay = "auto";
      }
      else
      {
        // TODO
      }
    }
    else if(auto m = ctre::match<"/speakers/correction/gain (.*)">(line))
    {
      if(m == "\"auto\"")
      {
        res.header.correction.gain = "auto";
      }
      else
      {
        // TODO
      }
    }
    else if(auto m = ctre::match<"/speakers/aed (.*)">(line))
    {
      int speaker_idx = 0;
      int i = 0;
      double a{}, e{}, d{};
      for(const auto word : std::views::split(m.get<1>().to_view(), ' '))
      {
        double result{};
        std::from_chars(word.data(), word.data() + word.size(), result);

        switch(i)
        {
          case 0:
            a = result;
            i++;
            break;
          case 1:
            e = result;
            i++;
            break;
          case 2: {
            d = result;
            i = 0;
            loudspeaker& spk = at_i(speaker_idx);
            spk.azimuth = a;
            spk.elevation = e;
            spk.distance = d;
            speaker_idx++;
            break;
          }
        }
      }
    }
  }

  return res;
}
catch(...)
{
  return std::nullopt;
}
}
