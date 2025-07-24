#include "spat_revolution_parser.hpp"

#include <boost/json.hpp>

#include <iostream>

namespace spatparse::spat_revolution
{

channel tag_invoke(boost::json::value_to_tag<channel>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  return channel{
      .name = boost::json::value_to<std::string>(obj.at("Name")),
      .type = obj.contains("Type") ? boost::json::value_to<std::string>(obj.at("Type")) : "",
      .azimuth = boost::json::value_to<double>(obj.at("Azimuth")),
      .elevation = boost::json::value_to<double>(obj.at("Elevation")),
      .distance = boost::json::value_to<double>(obj.at("Distance")),
      .yaw = boost::json::value_to<double>(obj.at("Yaw")),
      .pitch = boost::json::value_to<double>(obj.at("Pitch")),
      .roll = boost::json::value_to<double>(obj.at("Roll")),
      .index = boost::json::value_to<int>(obj.at("Index")),
      .connection_index = boost::json::value_to<int>(obj.at("ConnectionIndex")),
      .options = boost::json::value_to<int>(obj.at("Options"))
  };
}

configuration tag_invoke(boost::json::value_to_tag<configuration>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  
  std::vector<std::string> tags;
  if(obj.contains("tags") && obj.at("tags").is_array())
  {
    const auto& tags_array = obj.at("tags").as_array();
    for(const auto& tag : tags_array)
    {
      tags.push_back(boost::json::value_to<std::string>(tag));
    }
  }
  
  return configuration{
      .channels = boost::json::value_to<std::vector<channel>>(obj.at("Channels")),
      .uid = boost::json::value_to<std::string>(obj.at("UID")),
      .name = boost::json::value_to<std::string>(obj.at("Name")),
      .channel_desc = boost::json::value_to<std::string>(obj.at("ChannelDesc")),
      .stream_type = boost::json::value_to<std::string>(obj.at("StreamType")),
      .dimension = boost::json::value_to<int>(obj.at("Dimension")),
      .hoa_order = boost::json::value_to<int>(obj.at("HOAOrder")),
      .hoa_normalization = boost::json::value_to<std::string>(obj.at("HOANormalization")),
      .hoa_sorting = boost::json::value_to<std::string>(obj.at("HOASorting")),
      .rotation_x = boost::json::value_to<double>(obj.at("Rotation.x")),
      .rotation_y = boost::json::value_to<double>(obj.at("Rotation.y")),
      .rotation_z = boost::json::value_to<double>(obj.at("Rotation.z")),
      .rotation_w = boost::json::value_to<double>(obj.at("Rotation.w")),
      .is_user_config = boost::json::value_to<bool>(obj.at("IsUserConfig")),
      .tags = tags
  };
}

file tag_invoke(boost::json::value_to_tag<file>, const boost::json::value& jv)
{
  file result;
  
  if(jv.is_array())
  {
    result.configurations = boost::json::value_to<std::vector<configuration>>(jv);
  }
  else if(jv.is_object())
  {
    result.configurations.push_back(boost::json::value_to<configuration>(jv));
  }
  
  return result;
}

std::optional<file> parse(std::string_view json_string)
{
  try
  {
    boost::json::value jv = boost::json::parse(
        json_string, {},
        boost::json::parse_options{
            .allow_comments = true, .allow_trailing_commas = true});
    return boost::json::value_to<file>(jv);
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error parsing Spat Revolution file: " << e.what() << "\n";
    return std::nullopt;
  }
}

std::string to_string(const file& f)
{
  std::string json_string;
  json_string.reserve(65536);
  
  json_string += "[";
  
  for(std::size_t i = 0; i < f.configurations.size(); ++i)
  {
    const auto& config = f.configurations[i];
    json_string += std::format(
        "{{\n"
        "  \"Channels\": [\n");
    
    for(std::size_t j = 0; j < config.channels.size(); ++j)
    {
      const auto& ch = config.channels[j];
      json_string += std::format(
          "    {{\n"
          "      \"Name\": \"{}\",\n"
          "      \"Type\": \"{}\",\n"
          "      \"Azimuth\": {:.1f},\n"
          "      \"Elevation\": {:.1f},\n"
          "      \"Distance\": {:.1f},\n"
          "      \"Yaw\": {:.1f},\n"
          "      \"Pitch\": {:.1f},\n"
          "      \"Roll\": {:.1f},\n"
          "      \"Index\": {},\n"
          "      \"ConnectionIndex\": {},\n"
          "      \"Options\": {}\n"
          "    }}",
          ch.name, ch.type, ch.azimuth, ch.elevation, ch.distance,
          ch.yaw, ch.pitch, ch.roll, ch.index, ch.connection_index, ch.options);
      
      if(j < config.channels.size() - 1)
        json_string += ",\n";
      else
        json_string += "\n";
    }
    
    json_string += std::format(
        "  ],\n"
        "  \"UID\": \"{}\",\n"
        "  \"Name\": \"{}\",\n"
        "  \"ChannelDesc\": \"{}\",\n"
        "  \"StreamType\": \"{}\",\n"
        "  \"Dimension\": {},\n"
        "  \"HOAOrder\": {},\n"
        "  \"HOANormalization\": \"{}\",\n"
        "  \"HOASorting\": \"{}\",\n"
        "  \"Rotation.x\": {:.1f},\n"
        "  \"Rotation.y\": {:.1f},\n"
        "  \"Rotation.z\": {:.1f},\n"
        "  \"Rotation.w\": {:.1f},\n"
        "  \"IsUserConfig\": {},\n"
        "  \"tags\": [",
        config.uid, config.name, config.channel_desc, config.stream_type,
        config.dimension, config.hoa_order, config.hoa_normalization, config.hoa_sorting,
        config.rotation_x, config.rotation_y, config.rotation_z, config.rotation_w,
        config.is_user_config ? "true" : "false");
    
    for(std::size_t j = 0; j < config.tags.size(); ++j)
    {
      json_string += std::format("\"{}\"", config.tags[j]);
      if(j < config.tags.size() - 1)
        json_string += ", ";
    }
    
    json_string += "]\n}";
    
    if(i < f.configurations.size() - 1)
      json_string += ",\n";
    else
      json_string += "\n";
  }
  
  json_string += "]";
  
  return json_string;
}

}