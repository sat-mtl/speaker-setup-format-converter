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

  // Labels are not ASCII and not space-free. x3::print rejected every non-ASCII byte, and
  // without lexeme[] the skipper ate the spaces between the quotes, so a layout exported
  // with French or spaced speaker names could not be read back -- the export succeeded,
  // which is what made it quiet. Built from the parsed sample so every numeric field holds
  // something a writer would really emit; only the labels are swapped.
  {
    static const char* const labels[]
        = {"Caf\u00e9", "SPK 001", "\u0416", "\u65e5\u672c", "A B  C"};
    constexpr std::size_t n = std::size(labels);

    auto f = file;
    f.loudspeakers.resize(n, file.loudspeakers.front());
    for(std::size_t i = 0; i < n; i++)
    {
      f.loudspeakers[i].label = labels[i];
      f.loudspeakers[i].speaker = labels[i];
    }

    const auto text = spatparse::ease::to_string(f);
    const auto back = spatparse::ease::parse(text);
    if(!back)
    {
      std::cerr << "unicode/spaced labels: the file we just wrote does not parse\n";
      return 1;
    }
    if(back->loudspeakers.size() != n)
    {
      std::cerr << "unicode/spaced labels: " << back->loudspeakers.size()
                << " speakers, expected " << n << "\n";
      return 1;
    }
    for(std::size_t i = 0; i < n; i++)
    {
      if(back->loudspeakers[i].label != labels[i])
      {
        std::cerr << "unicode/spaced labels: wrote '" << labels[i] << "' read back '"
                  << back->loudspeakers[i].label << "'\n";
        return 1;
      }
    }

    // Windows exporters and most editors prepend a UTF-8 BOM.
    if(!spatparse::ease::parse("\xEF\xBB\xBF" + text))
    {
      std::cerr << "a leading UTF-8 BOM makes the file unparseable\n";
      return 1;
    }
  }

  return 0;
}