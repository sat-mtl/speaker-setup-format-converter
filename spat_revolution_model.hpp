#pragma once

#include <string>
#include <vector>

namespace spatparse::spat_revolution
{

struct channel
{
  std::string name;
  std::string type;
  double azimuth{};
  double elevation{};
  double distance{};
  double yaw{};
  double pitch{};
  double roll{};
  int index{};
  int connection_index{-1};
  int options{};
};

struct configuration
{
  std::vector<channel> channels;
  std::string uid;
  std::string name;
  std::string channel_desc;
  std::string stream_type;
  int dimension{};
  int hoa_order{};
  std::string hoa_normalization;
  std::string hoa_sorting;
  double rotation_x{};
  double rotation_y{};
  double rotation_z{};
  double rotation_w{};
  bool is_user_config{};
  std::vector<std::string> tags;
};

struct file
{
  std::vector<configuration> configurations;
};

}