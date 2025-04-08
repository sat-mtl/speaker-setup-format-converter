#pragma once

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <string>
#include <vector>

//////////////////////////////
// Our data model
//////////////////////////////
struct file_header
{
  std::string file_type;
  double format{};
  std::string length_unit;
};
BOOST_FUSION_ADAPT_STRUCT(file_header, file_type, format, length_unit)

struct loudspeaker
{
  std::string label;
  double x, y, z;
  double ver, hor, rot;
  std::string speaker;
  int delay;
  int align;
  std::vector<int> db_1m;
  int phase;
  int watts;
};
BOOST_FUSION_ADAPT_STRUCT(
    loudspeaker, label, x, y, z, ver, hor, rot, speaker, delay, align, db_1m, phase,
    watts)

struct file
{
  file_header header;
  std::vector<loudspeaker> loudspeakers;
};
BOOST_FUSION_ADAPT_STRUCT(file, header, loudspeakers)

using file_header_t = file_header;
using loudspeaker_t = loudspeaker;
using file_t = file;
