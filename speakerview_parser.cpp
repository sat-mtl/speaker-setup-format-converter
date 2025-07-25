#include "speakerview_parser.hpp"

#include "utils.hpp"

#include <boost/type_traits/copy_cv.hpp>

#include <pugixml.hpp>

#include <charconv>
#include <format>
#include <iostream>
namespace spatparse::spatgris
{

// Helper function to parse CARTESIAN_POSITION="(x, y, z)"
static bool
parse_cartesian_position(const std::string& pos_str, double& x, double& y, double& z)
{
  // Remove parentheses and spaces
  std::string clean_str = pos_str;
  clean_str.erase(std::remove(clean_str.begin(), clean_str.end(), '('), clean_str.end());
  clean_str.erase(std::remove(clean_str.begin(), clean_str.end(), ')'), clean_str.end());
  clean_str.erase(std::remove(clean_str.begin(), clean_str.end(), ' '), clean_str.end());

  // Split by comma
  size_t first_comma = clean_str.find(',');
  size_t second_comma = clean_str.find(',', first_comma + 1);

  if(first_comma == std::string::npos || second_comma == std::string::npos)
    return false;

  std::string x_str = clean_str.substr(0, first_comma);
  std::string y_str = clean_str.substr(first_comma + 1, second_comma - first_comma - 1);
  std::string z_str = clean_str.substr(second_comma + 1);

  try
  {
    x = std::stod(x_str);
    y = std::stod(y_str);
    z = std::stod(z_str);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

// Parse speakers from v4 format (recursive to handle groups)
static void parse_v4_speakers(pugi::xml_node root_node, std::vector<node>& speakers)
{
  for(pugi::xml_node child : root_node.children())
  {
    if(std::string_view(child.name()) == "SPEAKER")
    {
      const std::string_view state = child.attribute("IO_STATE").as_string("normal");
      if(state != "normal")
      {
        continue;
      }

      loudspeaker current_speaker;

      // Get name from SPEAKER_PATCH_ID
      int patch_id = child.attribute("SPEAKER_PATCH_ID").as_int(-1);
      if(patch_id >= 0)
      {
        current_speaker.name = std::format("SPEAKER_{}", patch_id);
      }

      // Parse CARTESIAN_POSITION
      std::string pos_str = child.attribute("CARTESIAN_POSITION").as_string();
      if(!parse_cartesian_position(
             pos_str, current_speaker.x, current_speaker.y, current_speaker.z))
      {
        std::cerr << "Warning: Failed to parse CARTESIAN_POSITION: " << pos_str
                  << std::endl;
        continue;
      }

      double gain_db = child.attribute("GAIN").as_double(0.0);
      current_speaker.gain = from_db(gain_db);

      speakers.push_back(current_speaker);
    }
    else if(std::string_view(child.name()) == "SPEAKER_GROUP")
    {
      // Recursively parse speakers in groups
      parse_v4_speakers(child, speakers);
    }
  }
}

static void parse_v3_speakers(pugi::xml_node root_node, std::vector<node>& speakers)
{
  // Parse v3 format (original implementation)
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

    spatparse::spatgris::loudspeaker current_speaker;
    current_speaker.name = speaker_node.name();

    current_speaker.x = pos_node.attribute("X").as_double();
    current_speaker.y = pos_node.attribute("Y").as_double();
    current_speaker.z = pos_node.attribute("Z").as_double();

    double gain_db = speaker_node.attribute("GAIN").as_double(0.0);
    current_speaker.gain = from_db(gain_db);

    speakers.push_back(current_speaker);
  }
}

template <typename T>
struct recurse_all_speakers
{
  using loudspeaker_t = boost::copy_cv_t<loudspeaker, T>;
  using group_t = boost::copy_cv_t<group, T>;
  using node_t = boost::copy_cv_t<node, T>;

  std::vector<loudspeaker_t*>& ret;

  void operator()(loudspeaker_t& l) { ret.push_back(&l); }
  void operator()(group_t& l)
  {
    for(auto& n : l.children)
      std::visit(*this, n);
  }
  void operator()(node_t& n) { std::visit(*this, n); }
};

template <typename F>
static auto do_all_speakers(F& f)
{
  using loudspeaker_t = boost::copy_cv_t<loudspeaker, F>;
  std::vector<loudspeaker_t*> ret;

  for(auto& node : f.children)
    recurse_all_speakers<F>{ret}(node);

  return ret;
}

std::vector<loudspeaker*> all_speakers(file& f)
{
  return do_all_speakers(f);
}
std::vector<const loudspeaker*> all_speakers(const file& f)
{
  return do_all_speakers(f);
}

std::optional<spatparse::spatgris::file> parse(std::string_view input)
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

  spatparse::spatgris::file output_file;

  // Check if this is v4 format by looking for SPEAKER_SETUP_VERSION attribute
  bool is_v4 = root_node.attribute("SPEAKER_SETUP_VERSION").as_string()[0] != '\0';

  if(is_v4)
  {
    // Parse v4 format
    parse_v4_speakers(root_node, output_file.children);
  }
  else
  {
    parse_v3_speakers(root_node, output_file.children);
  }

  return output_file;
}

void fixup(file& f, fixup_options opts)
{
  auto speakers = all_speakers(f);
  if(speakers.empty())
    return;

  auto s0 = *speakers[0];

  // Accumulators to compute the centroid
  double sx{}, sy{}, sz{};

  double min_x{s0.x}, min_y{s0.y}, min_z{s0.z};
  double max_x{s0.x}, max_y{s0.y}, max_z{s0.z};

  // 1. Basic cleanup
  int missing_idx{1};
  for(auto& sp_p : speakers)
  {
    auto& sp = *sp_p;
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
    const auto n = speakers.size();
    sx /= n;
    sy /= n;
    sz /= n;

    for(auto& sp_p : speakers)
    {
      auto& sp = *sp_p;
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
      for(auto& sp : speakers)
        sp->x *= *opts.normalize / (max_x - min_x);
    if(max_y > min_y)
      for(auto& sp : speakers)
        sp->y *= *opts.normalize / (max_y - min_y);
    if(max_z > min_z)
      for(auto& sp : speakers)
        sp->z *= *opts.normalize / (max_z - min_z);
  }
}

std::string to_string(const file& f)
{
  std::string res;
  res.append(R"_(<?xml version="1.0" encoding="UTF-8"?>
<SPEAKER_SETUP VERSION="3.1.14" SPAT_MODE="Dome" DIFFUSION="0.0" GENERAL_MUTE="0">
)_");

  int i = 1;
  for(auto* sp_p : all_speakers(f))
  {
    auto& sp = *sp_p;
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
