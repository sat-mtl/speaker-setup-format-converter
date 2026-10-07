#include "ease_parser.hpp"

#include <clocale>
#include <iostream>
#include <string>
#include <vector>

int main()
{
  static constexpr char sample_ease[] = {
#embed "ease_sample.ease"
  };

  std::string input = std::string(sample_ease, std::ssize(sample_ease));
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  // Parse
  auto maybe_file = spatparse::ease::parse(input);
  if(!maybe_file)
  {
    std::cerr << "Parse error.\n";
    return 1;
  }
  const auto& file = *maybe_file;

  // Print
  std::cout << file.header.file_type << "\n"
            << file.header.format << "\n"
            << file.header.length_unit << "\n";

  for(int i = 0; i < file.loudspeakers.size(); i++)
  {
    auto& l = file.loudspeakers[i];
    std::cout << i << ":\n"
              << l.label << "\n"
              << l.x << " " << l.y << " " << l.z << "\n"
              << l.ver << " " << l.hor << " " << l.rot << "\n"
              << l.speaker << "\n"
              << l.delay << "\n"
              << l.align << "\n";
    std::cout << "[";
    for(auto db : l.db_1m)
      std::cout << db << ",";

    std::cout << "]\n";
    std::cout << l.phase << "\n" << l.watts << "\n\n";
  }

  // What we write has to be what we can read: to_string() emits an accented copyright
  // comment, and an ASCII-only comment stripper used to make that output unparseable --
  // ease was the one format that could not round-trip through itself.
  auto reparsed = spatparse::ease::parse(spatparse::ease::to_string(file));
  if(!reparsed)
  {
    std::cerr << "Round-trip parse error.\n";
    return 1;
  }
  if(reparsed->loudspeakers.size() != file.loudspeakers.size())
  {
    std::cerr << "Round-trip lost speakers: " << reparsed->loudspeakers.size() << " != "
              << file.loudspeakers.size() << "\n";
    return 1;
  }

  return 0;
}
