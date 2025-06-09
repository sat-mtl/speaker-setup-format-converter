#pragma once
#include <string>
#include <variant>
#include <vector>

//////////////////////////////
// Our data model
//////////////////////////////
namespace spatparse::csv
{
struct xyz_position
{
  double x, y, z;
};
struct aed_position
{
  double a, e, d;
};

struct loudspeaker
{
  std::string name;
  std::variant<xyz_position, aed_position> position;
  double gain{1.0};
};

struct file
{
  std::vector<loudspeaker> speakers;
};
}
