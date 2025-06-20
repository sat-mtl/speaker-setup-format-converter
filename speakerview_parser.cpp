#include "speakerview_parser.hpp"

#include "utils.hpp"

#include <pugixml.hpp>

#include <format>
#include <iostream>
namespace spatparse::speakerview
{
std::optional<spatparse::speakerview::file> parse(std::string_view input)
{
  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer(input.data(), input.size());

  if(!result)
  {
    std::cerr << "XML parsing failed: " << result.description() << std::endl;
    return std::nullopt;
  }

  pugi::xml_node root_node = doc.child("SPEAKER_SETUP");
  if(!root_node)
  {
    std::cerr << "Error: Root node 'SPEAKER_SETUP' not found." << std::endl;
    return std::nullopt;
  }

  spatparse::speakerview::file output_file;

  for(pugi::xml_node speaker_node : root_node.children())
  {
    const std::string_view state = speaker_node.attribute("STATE").as_string("normal");
    if(state != "normal")
    {
      continue;
    }

    pugi::xml_node pos_node = speaker_node.child("POSITION");
    if(!pos_node)
    {
      std::cerr << "Warning: Speaker '" << speaker_node.name()
                << "' is missing a 'POSITION' node. Skipping." << std::endl;
      continue;
    }

    spatparse::speakerview::loudspeaker current_speaker;
    current_speaker.name = speaker_node.name();

    current_speaker.x = pos_node.attribute("X").as_double();
    current_speaker.y = pos_node.attribute("Y").as_double();
    current_speaker.z = pos_node.attribute("Z").as_double();

    double gain_db = speaker_node.attribute("GAIN").as_double(0.0);
    current_speaker.gain = from_db(gain_db);

    output_file.speakers.push_back(current_speaker);
  }

  return output_file;
}
void fixup(file& f, fixup_options opts)
{
  if(f.speakers.empty())
    return;
  auto s0 = f.speakers[0];

  // Accumulators to compute the centroid
  double sx{}, sy{}, sz{};

  double min_x{s0.x}, min_y{s0.y}, min_z{s0.z};
  double max_x{s0.x}, max_y{s0.y}, max_z{s0.z};

  // 1. Basic cleanup
  int missing_idx{1};
  for(auto& sp : f.speakers)
  {
    if(sp.name.empty())
      sp.name = std::format("missing_{}", missing_idx++);

    static constexpr auto epsilon = 1e-7;
    if(sp.x < epsilon)
      sp.x = 0.;
    if(sp.y < epsilon)
      sp.y = 0.;
    if(sp.z < epsilon)
      sp.z = 0.;
    if(sp.gain < epsilon)
      sp.gain = 0.;

    if(sp.x < min_x)
      min_x = sp.x;
    if(sp.y < min_y)
      min_y = sp.y;
    if(sp.z < min_z)
      min_z = sp.z;

    if(sp.x > max_x)
      max_x = sp.x;
    if(sp.y > max_y)
      max_y = sp.y;
    if(sp.z > max_z)
      max_z = sp.z;

    sx += sp.x;
    sy += sp.y;
    sz += sp.z;
  }

  // 2. Fixups
  if(opts.recenter)
  {
    const auto n = f.speakers.size();
    sx /= n;
    sy /= n;
    sz /= n;

    for(auto& sp : f.speakers)
    {
      sp.x -= sx;
      sp.y -= sy;
      sp.z -= sz;
    }

    min_x -= sx;
    max_x -= sx;
    min_y -= sy;
    max_y -= sy;
    min_z -= sz;
    max_z -= sz;
  }

  if(opts.normalize)
  {
    if(max_x > min_x)
      for(auto& sp : f.speakers)
        sp.x *= *opts.normalize / (max_x - min_x);
    if(max_y > min_y)
      for(auto& sp : f.speakers)
        sp.y *= *opts.normalize / (max_y - min_y);
    if(max_z > min_z)
      for(auto& sp : f.speakers)
        sp.z *= *opts.normalize / (max_z - min_z);
  }
}
std::string to_string(const file& f)
{
  std::string res;
  res.append(R"_(<?xml version="1.0" encoding="UTF-8"?>
<SPEAKER_SETUP VERSION="3.1.14" SPAT_MODE="Dome" DIFFUSION="0.0" GENERAL_MUTE="0">
)_");

  int i = 1;
  for(auto& sp : f.speakers)
  {
    res += std::format(
        R"_(  <SPEAKER_{} STATE="normal" GAIN="0.0" DIRECT_OUT_ONLY="0">
)_",
        i);
    res += std::format(
        R"_(    <POSITION X="{}" Y="{}" Z="{}"/>
)_",
        sp.x, sp.y, sp.z);
    res += std::format(
        R"_(  </SPEAKER_{}>
)_",
        i);
    i++;
  }

  res.append(R"_(</SPEAKER_SETUP>
)_");
  return res;
}
}
