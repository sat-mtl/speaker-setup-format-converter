#pragma once

#include "utils.hpp"

#include <array>
#include <iostream>
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

  // True if we know that this is an arrangement of loudspeakers on the shape of a sphere.
  // Enables for instance the better Dome algorithm in SpatGRIS.
  bool is_spherical{};

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
    return std::max(std::max(range_x, range_y), range_z);
  }

  void cleanup_small_values()
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
  }

  void check_spherical(double error_margin = 1e-2)
  {
    is_spherical = true;
    if(loudspeakers.size() < 4)
    {
      // There's always a sphere that can go through 3 points
      return;
    }

    double a{}, e{}, d{};
    cartesian_to_spherical(
        loudspeakers[0].x, loudspeakers[0].y, loudspeakers[0].z, a, e, d);
    for(auto& spk : loudspeakers)
    {
      double a2{}, e2{}, d2{};
      cartesian_to_spherical(spk.x, spk.y, spk.z, a2, e2, d2);

      if(std::abs(d - d2) > error_margin)
      {
        is_spherical = false;
      }
    }
  }

  void preprocess()
  {
    cleanup_small_values();

    check_spherical();

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
