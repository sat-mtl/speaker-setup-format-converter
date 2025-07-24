#include "converter.hpp"

#include "utils.hpp"

#include <cmath>

#include <format>
#include <variant>
namespace spatparse
{
void convert(
    const spatparse::aiira::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = input.header.name;
  output.description = input.header.description;
  output.length_unit = "m"; // AIIRA format implies meters for radius
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.layout.loudspeakers.size());

  for(const auto& spk_in : input.layout.loudspeakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = std::to_string(spk_in.channel);
    // Convert from IEM AIIRAD azimuth to standard azimuth before cartesian conversion
    double standard_azimuth = azimuth_from_iem_aiirad(spk_in.azimuth);
    spherical_to_cartesian(
        standard_azimuth, spk_in.elevation, spk_in.radius, spk_out.x, spk_out.y,
        spk_out.z);
    spk_out.gain_db = to_db(spk_in.gain);
    spk_out.is_virtual = spk_in.imaginary;
    spk_out.is_enabled = true; // No disabled state in this format
    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::csv::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = "CSV Imported Layout";
  output.description = "A loudspeaker layout imported from a CSV file.";
  // unit is unknown from format, default to meters
  output.length_unit = "m";
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.speakers.size());

  for(const auto& spk_in : input.speakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    spk_out.gain_db = to_db(spk_in.gain);

    std::visit([&](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr(std::is_same_v<T, spatparse::csv::xyz_position>)
      {
        spk_out.x = arg.x;
        spk_out.y = arg.y;
        spk_out.z = arg.z;
      }
      else if constexpr(std::is_same_v<T, spatparse::csv::aed_position>)
      {
        spherical_to_cartesian(arg.a, arg.e, arg.d, spk_out.x, spk_out.y, spk_out.z);
      }
    }, spk_in.position);

    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::ease::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = "EASE Focus 3 Export";
  output.description = "Layout exported from " + input.header.file_type;
  output.length_unit = input.header.length_unit;
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = spk_in.label;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.pitch = spk_in.ver;
    spk_out.yaw = spk_in.hor;
    spk_out.roll = spk_in.rot;
    spk_out.delay_ms = static_cast<double>(spk_in.delay);

    if(!spk_in.db_1m.empty())
    {
      spk_out.gain_db = static_cast<double>(spk_in.db_1m[0]);
    }

    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::spat::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = "Spat-Reaper Layout";
  output.description = "A layout from Spat";
  output.length_unit = "m"; // Spat typically uses meters for distance
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    // Convert from Spat5 azimuth to standard azimuth before cartesian conversion
    double standard_azimuth = azimuth_from_spat5(spk_in.azimuth);
    spherical_to_cartesian(
        standard_azimuth, spk_in.elevation, spk_in.distance, spk_out.x, spk_out.y,
        spk_out.z);
    spk_out.gain_db = spk_in.gain_db;
    spk_out.delay_ms = spk_in.delay;
    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::speakerview::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = "SpeakerView Layout";
  output.description = "A layout from SpeakerView";
  output.length_unit = "m"; // Unit is not specified, assume meters
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.speakers.size());

  for(const auto& spk_in : input.speakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.gain_db = to_db(spk_in.gain);
    output.loudspeakers.push_back(spk_out);
  }
}

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATIONS: Unified -> Specific
///////////////////////////////////////////////////////////////////////////////

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::aiira::file& output)
{
  output.header.name = input.name;
  output.header.description = input.description;
  output.layout.name = input.name;
  output.layout.loudspeakers.clear();
  output.layout.loudspeakers.reserve(input.loudspeakers.size());

  int channel_index = 1;
  for(const auto& spk_in : input.loudspeakers)
  {
    if(!spk_in.is_enabled)
      continue;

    spatparse::aiira::loudspeaker spk_out;
    double standard_azimuth, elevation, radius;
    cartesian_to_spherical(
        spk_in.x, spk_in.y, spk_in.z, standard_azimuth, elevation, radius);
    // Convert from standard azimuth to IEM AIIRAD azimuth
    spk_out.azimuth = azimuth_to_iem_aiirad(standard_azimuth);
    spk_out.elevation = elevation;
    spk_out.radius = radius;
    spk_out.channel = channel_index++;
    spk_out.gain = from_db(spk_in.gain_db);
    spk_out.imaginary = spk_in.is_virtual;
    output.layout.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::csv::file& output)
{
  output.speakers.clear();
  output.speakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    if(!spk_in.is_enabled)
      continue;

    spatparse::csv::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    spk_out.gain = from_db(spk_in.gain_db);

    // CSV can store either format, but unified only has Cartesian.
    // We will always write out Cartesian coordinates.
    spatparse::csv::xyz_position pos;
    pos.x = spk_in.x;
    pos.y = spk_in.y;
    pos.z = spk_in.z;
    spk_out.position = pos;

    output.speakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::ease::file& output)
{
  output.header.file_type = "loudspeakers";
  output.header.format = 4.1; // Default to a known format version
  output.header.length_unit = input.length_unit;
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    if(!spk_in.is_enabled)
      continue;

    spatparse::ease::loudspeaker spk_out;
    spk_out.label = spk_in.name;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.ver = spk_in.pitch;
    spk_out.hor = spk_in.yaw;
    spk_out.rot = spk_in.roll;
    spk_out.delay = static_cast<int>(std::round(spk_in.delay_ms));

    // EASE format has many fields that don't map from our simple model.
    // Set them to sensible defaults.
    spk_out.speaker = spk_in.name;
    spk_out.align = 0;
    spk_out.watts = 0;
    spk_out.phase = 0;
    spk_out.db_1m.clear();
    spk_out.db_1m.resize(21);

    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::spat::file& output)
{
  output.header.correction.delay = "auto"; // Default value
  output.header.correction.gain = "auto";  // Default value
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    if(!spk_in.is_enabled)
      continue;

    spatparse::spat::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    double standard_azimuth, elevation, distance;
    cartesian_to_spherical(
        spk_in.x, spk_in.y, spk_in.z, standard_azimuth, elevation, distance);
    // Convert from standard azimuth to Spat5 azimuth
    spk_out.azimuth = azimuth_to_spat5(standard_azimuth);
    spk_out.elevation = elevation;
    spk_out.distance = distance;
    spk_out.gain_db = spk_in.gain_db;
    spk_out.delay = spk_in.delay_ms;
    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::speakerview::file& output)
{
  output.speakers.clear();
  output.speakers.reserve(input.loudspeakers.size());

  for(const auto& spk_in : input.loudspeakers)
  {
    if(!spk_in.is_enabled)
      continue;

    spatparse::speakerview::loudspeaker spk_out;
    spk_out.name = spk_in.name;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.gain = from_db(spk_in.gain_db);
    output.speakers.push_back(spk_out);
  }
}

speakerview::file to_speakerview(ease::file in)
{
  speakerview::file res;
  for(const auto& sp_in : in.loudspeakers)
  {
    speakerview::loudspeaker sp_out;
    sp_out.name = sp_in.label;
    sp_out.gain = 1.0;
    sp_out.x = sp_in.x;
    sp_out.y = sp_in.y;
    sp_out.z = sp_in.z;
    res.speakers.push_back(sp_out);
  }
  return res;
}

speakerview::file to_speakerview(spat::file in)
{
  speakerview::file res;
  for(const auto& sp_in : in.loudspeakers)
  {
    speakerview::loudspeaker sp_out;
    sp_out.name = sp_in.name;
    sp_out.gain = sp_in.gain_db;
    // Convert from Spat5 azimuth to standard azimuth before cartesian conversion
    double standard_azimuth = azimuth_from_spat5(sp_in.azimuth);
    std::tie(sp_out.x, sp_out.y, sp_out.z)
        = aed_to_cartesian(standard_azimuth, sp_in.elevation, sp_in.distance);
    res.speakers.push_back(sp_out);
  }
  return res;
}

speakerview::file to_speakerview(aiira::file in)
{
  speakerview::file res;
  int i = 1;
  for(const auto& sp_in : in.layout.loudspeakers)
  {
    speakerview::loudspeaker sp_out;
    sp_out.name = std::format("{}", i);
    sp_out.gain = sp_in.gain;
    // Convert from IEM AIIRAD azimuth to standard azimuth before cartesian conversion
    double standard_azimuth = azimuth_from_iem_aiirad(sp_in.azimuth);
    std::tie(sp_out.x, sp_out.y, sp_out.z)
        = aed_to_cartesian(standard_azimuth, sp_in.elevation, sp_in.radius);
    res.speakers.push_back(sp_out);
    i++;
  }
  return res;
}

speakerview::file to_speakerview(csv::file in)
{
  speakerview::file res;
  if(in.speakers.empty())
    return res;
  if(in.speakers[0].position.index() == 0) // cartesian
  {
    for(const auto& sp_in : in.speakers)
    {
      speakerview::loudspeaker sp_out;
      sp_out.name = sp_in.name;
      sp_out.gain = 1.0;
      auto pos = std::get<csv::xyz_position>(in.speakers[0].position);
      sp_out.x = pos.x;
      sp_out.y = pos.y;
      sp_out.z = pos.z;
      res.speakers.push_back(sp_out);
    }
  }
  else if(in.speakers[0].position.index() == 1) // aed
  {
    for(const auto& sp_in : in.speakers)
    {
      speakerview::loudspeaker sp_out;
      sp_out.name = sp_in.name;
      sp_out.gain = 1.0;
      auto pos = std::get<csv::aed_position>(in.speakers[0].position);
      std::tie(sp_out.x, sp_out.y, sp_out.z) = aed_to_cartesian(pos.a, pos.e, pos.d);
      res.speakers.push_back(sp_out);
    }
  }
  return res;
}

void convert(
    const spatparse::fourdsound::file& input,
    spatparse::unified::loudspeaker_configuration& output)
{
  output.name = "4D Sound Layout";
  output.description = "A layout from 4D Sound";
  output.length_unit = "m"; // 4D Sound typically uses meters
  output.loudspeakers.clear();
  output.loudspeakers.reserve(input.speakers.size());

  for(const auto& spk_in : input.speakers)
  {
    spatparse::unified::loudspeaker spk_out;
    spk_out.name = spk_in.id;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.gain_db = 0.0; // No gain information in 4D Sound format
    spk_out.delay_ms = 0.0; // No delay information in 4D Sound format
    spk_out.is_enabled = true;
    spk_out.is_virtual = (spk_in.speakerType == "sub"); // Mark subs as virtual
    output.loudspeakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& input,
    spatparse::fourdsound::file& output)
{
  output.version = "2.1"; // Default to version 2.1
  output.speakers.clear();
  output.speakers.reserve(input.loudspeakers.size());

  int channel = 0;
  for(const auto& spk_in : input.loudspeakers)
  {
    spatparse::fourdsound::speaker spk_out;
    spk_out.id = spk_in.name.empty() ? std::format("speaker_{}", channel) : spk_in.name;
    spk_out.ch = channel++;
    spk_out.x = spk_in.x;
    spk_out.y = spk_in.y;
    spk_out.z = spk_in.z;
    spk_out.speakerType = spk_in.is_virtual ? "sub" : "satellite";
    output.speakers.push_back(spk_out);
  }
}

void convert(
    const spatparse::spat_revolution::file& in,
    spatparse::unified::loudspeaker_configuration& output)
{
  // If there are multiple configurations, use the first one
  if(in.configurations.empty())
    return;

  const auto& config = in.configurations[0];
  output.name = config.name;
  output.description = config.channel_desc;

  for(const auto& ch : config.channels)
  {
    spatparse::unified::loudspeaker sp_out;
    sp_out.name = ch.name;
    sp_out.gain_db = 1.0; // No gain information in Spat Revolution format

    // Convert spherical coordinates to Cartesian
    // Spat Revolution uses degrees for azimuth and elevation
    spherical_to_cartesian(
        ch.azimuth, ch.elevation, ch.distance, sp_out.x, sp_out.y, sp_out.z);

    output.loudspeakers.push_back(sp_out);
  }
}

void convert(
    const spatparse::unified::loudspeaker_configuration& in,
    spatparse::spat_revolution::file& output)
{
  spatparse::spat_revolution::configuration conf;
  conf.name = in.name;
  conf.channel_desc = in.description;

  for(const auto& sp : in.loudspeakers)
  {
    spatparse::spat_revolution::channel ch_out;
    ch_out.name = sp.name;
    cartesian_to_spherical(
        sp.x, sp.y, sp.z, ch_out.azimuth, ch_out.elevation, ch_out.distance);

    conf.channels.push_back(ch_out);
  }
}
}
