#include "csv_parser.hpp"

#include <clocale>
#include <iostream>

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
}
