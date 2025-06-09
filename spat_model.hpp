#pragma once
#include <string>
#include <vector>

//////////////////////////////
// Our data model
//////////////////////////////
namespace spatparse::spat
{

struct speakers
{
  struct
  {
    std::string delay;
    std::string gain;
  } correction;
};

struct loudspeaker
{
  std::string name;
  double azimuth{};
  double elevation{};
  double distance{};
  double delay{};
  double gain_db{};
};

struct file
{
  speakers header;
  std::vector<loudspeaker> loudspeakers;
};

}
