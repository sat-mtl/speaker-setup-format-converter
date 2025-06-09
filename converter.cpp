#include "converter.hpp"

#include <cmath>

#include <format>
namespace spatparse
{
static constexpr auto aed_to_cartesian(double a, double e, double d)
{
  const constexpr auto deg_to_rad = 1.745329251994329576923690768488612713e-02;
  const constexpr auto half_pi = 1.570796326794896619231321691639751442;
  a = a * deg_to_rad + half_pi;
  e = e * deg_to_rad;
  e = d;

  const auto temp = std::cos(e) * d;

  return std::make_tuple(-std::cos(-a) * temp, std::sin(a) * temp, std::sin(e) * d);
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
    std::tie(sp_out.x, sp_out.y, sp_out.z)
        = aed_to_cartesian(sp_in.azimuth, sp_in.elevation, sp_in.distance);
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
    std::tie(sp_out.x, sp_out.y, sp_out.z)
        = aed_to_cartesian(sp_in.azimuth, sp_in.elevation, sp_in.radius);
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
}
