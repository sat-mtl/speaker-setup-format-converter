#pragma once

#include <array>
#include <string>
#include <vector>

namespace spatparse::unified
{

struct loudspeaker
{
  std::string name;

  double x{};
  double y{};
  double z{};

  /** Follows the SpatGRIS convention: 0-360° **/
  double yaw{};
  double pitch{};
  double roll{};

  double gain_db{};
  double delay_ms{};

  bool is_virtual{false};
  bool is_enabled{true};
};

struct loudspeaker_configuration
{
  std::string name;
  std::string description;
  std::string length_unit{"m"};

  std::vector<loudspeaker> loudspeakers;

  double normalization_ratio = 1.0;

  double compute_normalization_ratio() const noexcept
  {
    double min_x{1e99}, max_x{-1e99};
    double min_y{1e99}, max_y{-1e99};
    double min_z{1e99}, max_z{-1e99};
    double range_x = 1e99;
    double range_y = 1e99;
    double range_z = 1e99;

    if(loudspeakers.empty())
    {
      min_x = 0.;
      min_y = 0.;
      min_z = 0.;
      max_x = 1.;
      max_y = 1.;
      max_z = 1.;
    }

    for(auto& sp : loudspeakers)
    {
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
    }

    range_x = std::max(std::abs(min_x), std::abs(max_x));
    range_y = std::max(std::abs(min_y), std::abs(max_y));
    range_z = std::max(std::abs(min_z), std::abs(max_z));
    fprintf(stderr, "Ayy: %f   %f   %f\n", range_x, range_y, range_z);
    return std::max(std::max(range_x, range_y), range_z);
  }

  void preprocess()
  {
    for(auto& sp : loudspeakers)
    {
      if(std::abs(sp.x) < 1e-10)
        sp.x = 0.;
      if(std::abs(sp.y) < 1e-10)
        sp.y = 0.;
      if(std::abs(sp.z) < 1e-10)
        sp.z = 0.;
      if(std::abs(sp.yaw) < 1e-10)
        sp.yaw = 0.;
      if(std::abs(sp.pitch) < 1e-10)
        sp.pitch = 0.;
      if(std::abs(sp.roll) < 1e-10)
        sp.roll = 0.;
    }

    if(length_unit == "")
      normalization_ratio = 1.0;
    else
      normalization_ratio = compute_normalization_ratio();
  }

  // meter to unit-less
  std::array<double, 3> normalize_distance(double x, double y, double z) const noexcept
  {
    if(length_unit == "")
      return {x, y, z};

    if(length_unit == "m")
    {
      if(normalization_ratio <= 0.)
        return {x, y, z};

      return {x / normalization_ratio, y / normalization_ratio, z / normalization_ratio};
    }

    return {x, y, z};
  }

  // unit-less to meter
  std::array<double, 3> scale_distance(
      double x, double y, double z, double scale_x, double scale_y,
      double scale_z) const noexcept
  {
    if(length_unit == "m")
      return {x, y, z};

    if(length_unit == "")
      return {x * scale_x, y * scale_y, z * scale_z};

    return {x, y, z};
  }

  std::array<double, 3> scale_distance(const loudspeaker& sp) const noexcept
  {
    return scale_distance(
        sp.x, sp.y, sp.z, normalization_ratio, normalization_ratio, normalization_ratio);
  }
};

}
