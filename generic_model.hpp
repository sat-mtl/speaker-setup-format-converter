#pragma once

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
};

}
