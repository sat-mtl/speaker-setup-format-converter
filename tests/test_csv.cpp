#include "csv_parser.hpp"

#include <clocale>
#include <iostream>
#include <string>

static constexpr char sample_csv[] = {
#embed "csv_sample.csv"
};

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  auto res = spatparse::csv::parse(std::string_view(sample_csv, std::ssize(sample_csv)));
  if(!res)
  {
    std::cerr << "Parsing failed\n";
    return 1;
  }

  for(auto sp : res->speakers)
  {
    std::cerr << sp.name << "\n";
    if(auto pos = std::get_if<spatparse::csv::xyz_position>(&sp.position))
    {
      std::cerr << " => XYZ: " << pos->x << ", " << pos->y << ", " << pos->z << "\n";
    }
    else if(auto pos = std::get_if<spatparse::csv::aed_position>(&sp.position))
    {
      std::cerr << " => AED: " << pos->a << ", " << pos->e << ", " << pos->d << "\n";
    }
  }

  // A row with fewer cells than the header leaves the columns it never reaches untouched.
  // xyz_position used to declare "double x, y, z;" with no initializer, so those columns
  // read back whatever the previous row had left in that stack slot -- here the short row
  // came out with z = 333, silently inheriting the speaker above it.
  {
    const auto ragged = spatparse::csv::parse("names,x,y,z\nfull,111,222,333\nshort,7,8\n");
    if(!ragged || ragged->speakers.size() != 2)
    {
      std::cerr << "ragged row: expected 2 speakers, got "
                << (ragged ? std::to_string(ragged->speakers.size()) : "a parse failure")
                << "\n";
      return 1;
    }
    const auto* pos = std::get_if<spatparse::csv::xyz_position>(&ragged->speakers[1].position);
    if(!pos)
    {
      std::cerr << "ragged row: not an xyz position\n";
      return 1;
    }
    if(pos->z != 0.)
    {
      std::cerr << "ragged row: z is " << pos->z << ", expected 0 -- it kept the previous"
                   " row's value\n";
      return 1;
    }
  }

  return 0;
}