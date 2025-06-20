#pragma once
#include <cmath>

#include <numbers>
#include <tuple>

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

// Converts degrees to radians
static constexpr double to_radians(double degrees)
{
  return degrees * std::numbers::pi / 180.0;
}

// Converts radians to degrees
static constexpr double to_degrees(double radians)
{
  return radians * 180.0 / std::numbers::pi;
}

// Converts a linear gain factor to decibels (dB)
// Handles non-positive gain values to avoid log(-inf) or NaN.
static double to_db(double linear_gain)
{
  if(linear_gain <= 0.0)
  {
    return -INFINITY; // Or a very large negative number
  }
  return 20.0 * std::log10(linear_gain);
}

// Converts decibels (dB) to a linear gain factor
static double from_db(double db_gain)
{
  return std::pow(10.0, db_gain / 20.0);
}

// Helper to convert spherical coordinates (in degrees) to Cartesian
static void spherical_to_cartesian(
    double azimuth_deg, double elevation_deg, double radius, double& x, double& y,
    double& z)
{
  const double az_rad = to_radians(azimuth_deg);
  const double el_rad = to_radians(elevation_deg);
  x = radius * std::cos(el_rad) * std::cos(az_rad);
  y = radius * std::cos(el_rad) * std::sin(az_rad);
  z = radius * std::sin(el_rad);
}

// Helper to convert Cartesian coordinates to spherical (in degrees)
static void cartesian_to_spherical(
    double x, double y, double z, double& azimuth_deg, double& elevation_deg,
    double& radius)
{
  radius = std::sqrt(x * x + y * y + z * z);
  if(radius == 0.0)
  {
    azimuth_deg = 0.0;
    elevation_deg = 0.0;
  }
  else
  {
    azimuth_deg = to_degrees(std::atan2(y, x));
    elevation_deg = to_degrees(std::asin(z / radius));
  }
}

}
