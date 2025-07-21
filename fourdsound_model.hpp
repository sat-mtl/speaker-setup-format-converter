#pragma once

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <string>
#include <vector>
#include <optional>

//////////////////////////////
// 4D Sound data model
//////////////////////////////
namespace spatparse::fourdsound
{

struct speaker
{
  std::string id;
  int ch;
  double x, y, z;
  std::string speakerType;
};

struct shape
{
  std::string type;
  std::string speakers; // Space-separated speaker IDs
};

struct grid
{
  std::string name;
  std::optional<std::string> speakerType;
  std::vector<shape> shapes;
  std::optional<double> projectionPoint_x;
  std::optional<double> projectionPoint_y;
  std::optional<double> projectionPoint_z;
};

struct group
{
  std::string name;
  std::string channels; // Space-separated channel numbers
};

struct wall
{
  std::string name;
  std::string channels; // Space-separated channel numbers
};

struct routing
{
  std::string value; // Space-separated routing values
};

struct file
{
  std::optional<std::string> version;
  std::vector<speaker> speakers;
  std::vector<grid> grids;
  std::vector<group> groups;
  std::vector<wall> walls;
  std::optional<routing> routing_info;
};

using speaker_t = speaker;
using shape_t = shape;
using grid_t = grid;
using group_t = group;
using wall_t = wall;
using routing_t = routing;
using file_t = file;

}

BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::speaker, id, ch, x, y, z, speakerType)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::shape, type, speakers)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::grid, name, speakerType, shapes, projectionPoint_x, projectionPoint_y, projectionPoint_z)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::group, name, channels)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::wall, name, channels)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::routing, value)
BOOST_FUSION_ADAPT_STRUCT(spatparse::fourdsound::file, version, speakers, grids, groups, walls, routing_info)