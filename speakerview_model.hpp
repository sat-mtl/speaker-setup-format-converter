#pragma once
#include <string>
#include <vector>

namespace spatparse::speakerview
{

struct loudspeaker
{
  std::string name;
  double x{}, y{}, z{};
  double gain{1.0};
};

struct file
{
  std::vector<loudspeaker> speakers;
};

}
