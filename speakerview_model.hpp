#pragma once
#include <string>
#include <variant>
#include <vector>

namespace spatparse::spatgris
{

struct loudspeaker;
struct group;
using node = std::variant<group, loudspeaker>;

struct loudspeaker
{
  std::string name;
  std::string uuid;
  double x{}, y{}, z{};

  double gain{1.0};
};

struct group
{
  std::string name;
  std::string uuid;
  double x{}, y{}, z{};

  std::vector<node> children;
};

struct file
{
  std::string mode = "Cube";
  double diffusion{};
  double general_mute{};

  std::vector<node> children;
};

}
