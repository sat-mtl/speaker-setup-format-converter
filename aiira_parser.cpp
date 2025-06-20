#include "aiira_parser.hpp"

#include <boost/json.hpp>
#include <boost/json/src.hpp>

#include <iostream>
namespace spatparse::aiira
{
/**
 * @brief Converts a JSON object to a `loudspeaker`.
 *
 * This function is found by `value_to` via argument-dependent lookup (ADL).
 * It maps the JSON keys to the struct members.
 * Note the mapping from "IsImaginary" in JSON to "imaginary" in the struct.
 */
loudspeaker
tag_invoke(boost::json::value_to_tag<loudspeaker>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  return loudspeaker{
      .azimuth = boost::json::value_to<double>(obj.at("Azimuth")),
      .elevation = boost::json::value_to<double>(obj.at("Elevation")),
      .radius = boost::json::value_to<double>(obj.at("Radius")),
      .imaginary
      = boost::json::value_to<bool>(obj.at("IsImaginary")), // Name mismatch handled here
      .channel = boost::json::value_to<int>(obj.at("Channel")),
      .gain = boost::json::value_to<double>(obj.at("Gain"))};
}

/**
 * @brief Converts a JSON object to a `loudspeaker_layout`.
 *
 * `value_to` for `std::vector<loudspeaker>` will automatically call the
 * `tag_invoke` overload for `loudspeaker` for each element in the JSON array.
 */
loudspeaker_layout
tag_invoke(boost::json::value_to_tag<loudspeaker_layout>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  return loudspeaker_layout{
      .name = boost::json::value_to<std::string>(obj.at("Name")),
      .loudspeakers
      = boost::json::value_to<std::vector<loudspeaker>>(obj.at("Loudspeakers"))};
}

/**
 * @brief Converts a JSON object to a `file_header`.
 *
 * This overload is designed to extract header information from the top-level
 * JSON object.
 */
file_header
tag_invoke(boost::json::value_to_tag<file_header>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  return file_header{
      .name = boost::json::value_to<std::string>(obj.at("Name")),
      .description = boost::json::value_to<std::string>(obj.at("Description"))};
}
/**
 * @brief Converts the top-level JSON object to a `file`.
 *
 * This function orchestrates the parsing by delegating to the other `tag_invoke`
 * overloads. It reuses the top-level JSON value `jv` for the `file_header`
 * and extracts the "LoudspeakerLayout" sub-object for the `loudspeaker_layout`.
 */
file tag_invoke(boost::json::value_to_tag<file>, const boost::json::value& jv)
{
  const boost::json::object& obj = jv.as_object();
  return file{
      .header = boost::json::value_to<file_header>(jv), // Re-use the top-level object
      .layout = boost::json::value_to<loudspeaker_layout>(obj.at("LoudspeakerLayout"))};
}

/**
 * @brief Main parsing function for an AIIRA file.
 *
 * @param json_string A string view containing the JSON data.
 * @return The parsed `spatparse::aiira::file` object.
 * @throws std::runtime_error on parsing or conversion failure.
 */
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
    std::cerr << e.what() << "\n";
    return std::nullopt;
  }
}

std::string to_string(const spatparse::aiira::file& f)
{
  std::string json_string;
  json_string.reserve(32768);

  json_string += std::format(
      "{{\n"
      "  \"Name\": \"{}\",\n"
      "  \"Description\": \"{}\",\n"
      "  \"LoudspeakerLayout\": {{\n"
      "    \"Name\": \"{}\",\n"
      "    \"Loudspeakers\": [\n",
      f.header.name, f.header.description, f.layout.name);

  for(std::size_t i = 0; i < f.layout.loudspeakers.size(); ++i)
  {
    const auto& spk = f.layout.loudspeakers[i];
    json_string += std::format(
        "      {{\n"
        "        \"Azimuth\": {:.6f},\n"
        "        \"Elevation\": {:.6f},\n"
        "        \"Radius\": {:.6f},\n"
        "        \"IsImaginary\": {},\n"
        "        \"Channel\": {},\n"
        "        \"Gain\": {:.6f}\n"
        "      }}",
        spk.azimuth, spk.elevation, spk.radius, spk.imaginary ? "true" : "false",
        spk.channel, spk.gain);

    if(i < f.layout.loudspeakers.size() - 1)
      json_string += ",\n";
    else
      json_string += "\n";
  }

  json_string +=
      "    ]\n"
      "  }\n"
      "}\n";

  return json_string;
}

} // namespace spatparse::aiira
