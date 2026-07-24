#include "spat_parser.hpp"

#include "utils.hpp"

#include <ctre.hpp>

#include <format>
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

  // Speaker indices and counts come straight out of the file: bound them, or a
  // malformed one sends resize() / reserve() after an absurd allocation.
  constexpr int max_loudspeakers = 65535;

  auto at_i = [&](int i) -> loudspeaker* {
    if(i < 0 || i > max_loudspeakers)
      return nullptr;

    auto& vec = res.loudspeakers;
    if(std::ssize(vec) < i + 1)
    {
      vec.resize(i + 1);
    }
    return &vec[i];
  };

  for(const auto word : std::views::split(input, '\n'))
  {
    std::string_view line = strip_cr(std::string_view{word});
    if(line.ends_with('\\') || line.ends_with('}'))
      line = line.substr(0, line.size() - 1);

    if(auto m = ctre::match<"/speaker/number (.*)">(line))
    {
      const int speaker_count = m.get<1>().to_number();
      if(speaker_count > 0 && speaker_count <= max_loudspeakers)
        res.loudspeakers.reserve(speaker_count);
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/delay (.*)">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      if(const auto delay = spatparse::parse_strict<double>(m.get<2>().to_view()))
        if(auto* spk = at_i(speaker_idx))
          spk->delay = *delay;
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/gain/db (.*)">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      if(const auto gain_db = spatparse::parse_strict<double>(m.get<2>().to_view()))
        if(auto* spk = at_i(speaker_idx))
          spk->gain_db = *gain_db;
    }
    else if(auto m = ctre::match<"/speaker/([0-9]+)/name \"(.*)\"">(line))
    {
      const int speaker_idx = m.get<1>().to_number() - 1;
      const auto name = m.get<2>().to_view();

      if(auto* spk = at_i(speaker_idx))
        spk->name = name;
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
        auto res = parse_strict<double>(std::string_view{word});
        if(!res)
          return std::nullopt;
        double result = *res;

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
            loudspeaker* spk = at_i(speaker_idx);
            if(!spk)
              return std::nullopt;
            spk->azimuth = a;
            spk->elevation = e;
            spk->distance = d;
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

std::string to_string(const spatparse::spat::file& f)
{
  std::string spat_string = R"_({\rtf1\ansi\ansicpg1252\cocoartf2639
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww10440\viewh12340\viewkind0
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0

\f0\fs24 \cf0 )_";
  spat_string.reserve(f.loudspeakers.size() * 128 + 256);

  spat_string += std::format(
      "/speaker/number {}\\\n"
      "/speakers/correction/delay \"{}\"\\\n"
      "/speakers/correction/gain \"{}\"\\\n"
      "/speakers/aed",
      f.loudspeakers.size(), f.header.correction.delay, f.header.correction.gain);

  for(const auto& spk : f.loudspeakers)
  {
    spat_string += std::format(
        " {:.6f} {:.6f} {:.6f}", spk.azimuth, spk.elevation, spk.distance);
  }
  spat_string += "\\\n";

  for(size_t i = 0; i < f.loudspeakers.size(); ++i)
  {
    spat_string
        += std::format("/speaker/{}/delay {:.6f}\\\n", i + 1, f.loudspeakers[i].delay);
  }
  for(size_t i = 0; i < f.loudspeakers.size(); ++i)
  {
    spat_string += std::format(
        "/speaker/{}/gain/db {:.6f}\\\n", i + 1, f.loudspeakers[i].gain_db);
  }
  for(size_t i = 0; i < f.loudspeakers.size(); ++i)
  {
    spat_string
        += std::format("/speaker/{}/name \"{}\"\\\n", i + 1, f.loudspeakers[i].name);
  }

  spat_string.pop_back();
  spat_string.push_back('}');
  return spat_string;
}
}
