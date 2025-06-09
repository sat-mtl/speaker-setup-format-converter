#pragma once

#include <string>
#include <vector>

//////////////////////////////
// Our data model
//////////////////////////////
namespace spatparse::aiira
{
struct file_header
{
  std::string name;
  std::string description;
};

struct loudspeaker
{
  double azimuth{};
  double elevation{};
  double radius{};
  bool imaginary{};
  int channel{}; // 1-based it seems?
  double gain{1.0};
};

struct loudspeaker_layout
{
  std::string name;
  std::vector<loudspeaker> loudspeakers;
};

struct file
{
  file_header header;
  loudspeaker_layout layout;
};
}
