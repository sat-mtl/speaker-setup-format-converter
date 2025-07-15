#pragma once
#include <cmath>

#if __has_include(<charconv>) && defined(__cpp_lib_to_chars)
#include <charconv>
#else
#include <boost/lexical_cast.hpp>
#endif
#include <numbers>
#include <optional>
#include <string_view>
#include <tuple>

namespace spatparse
{

template <typename T>
static std::optional<T> parse_strict(std::string_view instance) noexcept
{
  T n{};
#if defined(__cpp_lib_to_chars)
  const auto begin = instance.data();
  const auto end = instance.data() + instance.size();
  const auto [ptr, ec] = std::from_chars(begin, end, n);
  return (ec == std::errc{} && ptr == end) ? std::optional<T>{n} : std::nullopt;
#else
  if(boost::conversion::detail::try_lexical_convert(instance, n))
    return n;
  return std::nullopt;
#endif
}
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

// Convert azimuth from standard range [0, 360) to IEM AIIRAD range [-180, 180]
// IEM AIIRAD: negative values on the right (0 to 180 becomes 0 to -180)
//              positive values on the left (180 to 360 becomes 180 to 0)
static double azimuth_to_iem_aiirad(double azimuth)
{
  // Normalize to [0, 360) range
  while(azimuth < 0.0)
    azimuth += 360.0;
  while(azimuth >= 360.0)
    azimuth -= 360.0;

  if(azimuth <= 180.0)
  {
    // Right side: 0 to 180 becomes 0 to -180
    return -azimuth;
  }
  else
  {
    // Left side: 180 to 360 becomes 180 to 0
    return 360.0 - azimuth;
  }
}

// Convert azimuth from IEM AIIRAD range [-180, 180] to standard range [0, 360)
static double azimuth_from_iem_aiirad(double azimuth)
{
  if(azimuth <= 0.0)
  {
    // Right side: 0 to -180 becomes 0 to 180
    return -azimuth;
  }
  else
  {
    // Left side: 180 to 0 becomes 180 to 360
    return 360.0 - azimuth;
  }
}

// Convert azimuth from standard range [0, 360) to Spat5 range [-180, 180]
// Spat5: positive values on the right (0 to 180 stays 0 to 180)
//        negative values on the left (180 to 360 becomes -180 to 0)
static double azimuth_to_spat5(double azimuth)
{
  // Normalize to [0, 360) range
  while(azimuth < 0.0)
    azimuth += 360.0;
  while(azimuth >= 360.0)
    azimuth -= 360.0;

  if(azimuth <= 180.0)
  {
    // Right side: stays the same
    return azimuth;
  }
  else
  {
    // Left side: 180 to 360 becomes -180 to 0
    return azimuth - 360.0;
  }
}

// Convert azimuth from Spat5 range [-180, 180] to standard range [0, 360)
static double azimuth_from_spat5(double azimuth)
{
  if(azimuth < 0.0)
  {
    // Left side: -180 to 0 becomes 180 to 360
    return azimuth + 360.0;
  }
  else
  {
    // Right side: stays the same
    return azimuth;
  }
}

}
