#include "fourdsound_parser.hpp"

#include <pugixml.hpp>
#include <iostream>
#include <format>
#include <sstream>

namespace spatparse::fourdsound
{

std::optional<spatparse::fourdsound::file> parse(std::string_view input)
{
  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer(input.data(), input.size());

  if(!result)
  {
    std::cerr << "XML parsing failed: " << result.description() << std::endl;
    return std::nullopt;
  }

  pugi::xml_node root_node = doc.child("setup");
  if(!root_node)
  {
    std::cerr << "Error: Root node 'setup' not found." << std::endl;
    return std::nullopt;
  }

  spatparse::fourdsound::file output_file;

  // Parse version attribute if present
  if(auto version_attr = root_node.attribute("version"))
  {
    output_file.version = version_attr.as_string();
  }

  // Parse speakers
  for(pugi::xml_node speaker_node : root_node.children("speaker"))
  {
    spatparse::fourdsound::speaker current_speaker;
    
    current_speaker.id = speaker_node.attribute("id").as_string();
    current_speaker.ch = speaker_node.attribute("ch").as_int();
    current_speaker.x = speaker_node.attribute("x").as_double();
    current_speaker.y = speaker_node.attribute("y").as_double();
    current_speaker.z = speaker_node.attribute("z").as_double();
    current_speaker.speakerType = speaker_node.attribute("speakerType").as_string();

    output_file.speakers.push_back(current_speaker);
  }

  // Parse grids
  for(pugi::xml_node grid_node : root_node.children("grid"))
  {
    spatparse::fourdsound::grid current_grid;
    
    current_grid.name = grid_node.attribute("name").as_string();
    
    if(auto speaker_type_attr = grid_node.attribute("speakerType"))
    {
      current_grid.speakerType = speaker_type_attr.as_string();
    }

    // Parse shapes within grid
    for(pugi::xml_node shape_node : grid_node.children("shape"))
    {
      spatparse::fourdsound::shape current_shape;
      current_shape.type = shape_node.attribute("type").as_string();
      current_shape.speakers = shape_node.attribute("speakers").as_string();
      current_grid.shapes.push_back(current_shape);
    }

    // Parse projectionPoint if present
    if(pugi::xml_node proj_node = grid_node.child("projectionPoint"))
    {
      current_grid.projectionPoint_x = proj_node.attribute("x").as_double();
      current_grid.projectionPoint_y = proj_node.attribute("y").as_double();
      current_grid.projectionPoint_z = proj_node.attribute("z").as_double();
    }

    output_file.grids.push_back(current_grid);
  }

  // Parse groups
  for(pugi::xml_node group_node : root_node.children("group"))
  {
    spatparse::fourdsound::group current_group;
    current_group.name = group_node.attribute("name").as_string();
    current_group.channels = group_node.attribute("channels").as_string();
    output_file.groups.push_back(current_group);
  }

  // Parse walls
  for(pugi::xml_node wall_node : root_node.children("wall"))
  {
    spatparse::fourdsound::wall current_wall;
    current_wall.name = wall_node.attribute("name").as_string();
    current_wall.channels = wall_node.attribute("channels").as_string();
    output_file.walls.push_back(current_wall);
  }

  // Parse routing
  if(pugi::xml_node routing_node = root_node.child("routing"))
  {
    spatparse::fourdsound::routing routing_info;
    routing_info.value = routing_node.attribute("value").as_string();
    output_file.routing_info = routing_info;
  }

  return output_file;
}

std::string to_string(const file& f)
{
  std::stringstream ss;
  
  ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  
  if(f.version.has_value())
    ss << "<setup version=\"" << *f.version << "\">\n\n";
  else
    ss << "<setup>\n\n";

  // Write speakers
  for(const auto& speaker : f.speakers)
  {
    ss << std::format("	<speaker id=\"{}\" ch=\"{}\" x=\"{}\" y=\"{}\" z=\"{}\" speakerType=\"{}\" />\n",
                     speaker.id, speaker.ch, speaker.x, speaker.y, speaker.z, speaker.speakerType);
  }

  if(!f.speakers.empty())
    ss << "\n";

  // Write grids
  for(const auto& grid : f.grids)
  {
    if(grid.speakerType.has_value())
      ss << std::format("	<grid name=\"{}\" speakerType=\"{}\">\n", grid.name, *grid.speakerType);
    else
      ss << std::format("	<grid name=\"{}\">\n", grid.name);

    for(const auto& shape : grid.shapes)
    {
      ss << std::format("		<shape type=\"{}\" speakers=\"{}\" />\n", shape.type, shape.speakers);
    }

    if(grid.projectionPoint_x.has_value())
    {
      ss << std::format("		<projectionPoint x=\"{}\" y=\"{}\" z=\"{}\" />\n", 
                       *grid.projectionPoint_x, *grid.projectionPoint_y, *grid.projectionPoint_z);
    }

    ss << "	</grid>\n\n";
  }

  // Write groups
  for(const auto& group : f.groups)
  {
    ss << std::format("	<group name=\"{}\" channels=\"{}\" />\n", group.name, group.channels);
  }

  if(!f.groups.empty())
    ss << "\n";

  // Write walls
  for(const auto& wall : f.walls)
  {
    ss << std::format("	<wall name=\"{}\" channels=\"{}\" />\n", wall.name, wall.channels);
  }

  if(!f.walls.empty())
    ss << "\n";

  // Write routing
  if(f.routing_info.has_value())
  {
    ss << std::format("	<routing value=\"{}\" />\n\n", f.routing_info->value);
  }

  ss << "</setup>\n";

  return ss.str();
}

}