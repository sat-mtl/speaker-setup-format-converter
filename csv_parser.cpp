#include "csv_parser.hpp"

#include "utils.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>

#if __has_include(<csv2/reader.hpp>)
#include <csv2/reader.hpp>
#else
#include <csv2/csv2.hpp>
#endif

#include <iostream>

namespace spatparse::csv
{
// csv2 trims spaces and tabs by default, which leaves the '\r' of a CRLF file
// at the end of the last cell of every row.
using reader = csv2::Reader<
    csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>,
    csv2::trim_policy::trim_characters<' ', '\t', '\r'>>;

std::optional<file> parse(std::string_view input)
try
{
  file res;

  reader r;

  r.parse_view(input);

  int columns = r.cols();
  std::optional<int> col_name;
  std::optional<int> col_gain;
  std::optional<int> col_x;
  std::optional<int> col_y;
  std::optional<int> col_z;

  std::optional<int> col_a;
  std::optional<int> col_e;
  std::optional<int> col_d;

  int col_index = 0;
  for(auto k : r.header())
  {
    std::string name{k.read_view()};
    boost::trim(name);
    boost::to_lower(name);

    if(name.contains("name"))
      col_name = col_index;
    else if(name.contains("gain"))
      col_gain = col_index;
    else if(name.contains("x"))
      col_x = col_index;
    else if(name.contains("y"))
      col_y = col_index;
    else if(name.contains("z"))
      col_z = col_index;
    else if(name.contains("a"))
      col_a = col_index;
    else if(name.contains("e"))
      col_e = col_index;
    else if(name.contains("d"))
      col_d = col_index;
    col_index++;
  }

  const bool has_xyz = (col_x && col_y && col_z);
  const bool has_aed = (col_a && col_e && col_d);

  if(!(has_xyz || has_aed))
  {
    std::cerr << "Cannot find meaningful data \n";
    return std::nullopt;
  }

  std::string v;
  for(const reader::Row& row : r)
  {
    if(row.length() == 0)
      continue;
    loudspeaker sp;

    int column = 0;
    xyz_position xyz;
    aed_position aed;
    for(const auto& cell : row)
    {
      v.clear();
      cell.read_value(v);

      if(column == col_name)
      {
        sp.name = std::string(v);
      }
      else
      {
        auto res = parse_strict<double>(v);
        if(!res)
          return std::nullopt;
        double num = *res;

        if(column == col_x)
          xyz.x = num;
        else if(column == col_y)
          xyz.y = num;
        else if(column == col_z)
          xyz.z = num;
        else if(column == col_a)
          aed.a = num;
        else if(column == col_e)
          aed.e = num;
        else if(column == col_d)
          aed.d = num;
        else if(column == col_gain)
          sp.gain = num;
      }

      column++;
    }
    if(has_xyz)
      sp.position = xyz;
    else if(has_aed)
      sp.position = aed;

    res.speakers.push_back(sp);
  }

  return res;
}
catch(...)
{
  std::cerr << "Parse error!\n ";
  return std::nullopt;
}

std::string to_string(const spatparse::csv::file& f)
{
  if(f.speakers.empty())
  {
    return "";
  }

  std::string csv_string;
  csv_string.reserve(f.speakers.size() * 64); // Pre-allocate

  std::visit([&csv_string](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr(std::is_same_v<T, spatparse::csv::xyz_position>)
    {
      csv_string += "names,x,y,z\n";
    }
    else if constexpr(std::is_same_v<T, spatparse::csv::aed_position>)
    {
      csv_string += "names,azimuth,elevation,distance\n";
    }
  }, f.speakers.front().position);

  for(const auto& speaker : f.speakers)
  {
    std::visit([&](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr(std::is_same_v<T, spatparse::csv::xyz_position>)
      {
        csv_string += std::format(
            "{},{:.4f},{:.4f},{:.4f}\n", speaker.name, arg.x, arg.y, arg.z);
      }
      else if constexpr(std::is_same_v<T, spatparse::csv::aed_position>)
      {
        csv_string += std::format(
            "{},{:.4f},{:.4f},{:.4f}\n", speaker.name, arg.a, arg.e, arg.d);
      }
    }, speaker.position);
  }

  return csv_string;
}
}
