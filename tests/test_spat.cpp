#include "spat_parser.hpp"

#include <clocale>
#include <iostream>

static constexpr char sample_rtf[] = {
#embed "spat_panoramix_sample.rtf"
};

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");
  auto res
      = spatparse::spat::parse(std::string_view(sample_rtf, std::ssize(sample_rtf)));
  if(!res)
  {
    std::cerr << "Parsing failed\n";
    return 1;
  }

  for(const auto& spk : res->loudspeakers)
  {
    std::cerr << "Speaker: " << spk.name << "\n";
    std::cerr << " => AED: " << spk.azimuth << ", " << spk.elevation << ", "
              << spk.distance << "\n";
    std::cerr << " => GAIN: " << spk.gain_db << "\n";
    std::cerr << " => DELAY: " << spk.delay << "\n\n";
  }

  // What we write has to come back intact, the last line included. to_string() keeps every
  // line's '\\' continuation and turns the final newline into '}', so the file ends
  // "...\"Last\"\\}". The parser stripped a single trailing character, which left the '\\'
  // glued to the quoted value and made the name pattern miss -- the last speaker, and only
  // the last, came back nameless from every round-trip.
  {
    const auto back = spatparse::spat::parse(spatparse::spat::to_string(*res));
    if(!back)
    {
      std::cerr << "round-trip: the file we just wrote does not parse\n";
      return 1;
    }
    if(back->loudspeakers.size() != res->loudspeakers.size())
    {
      std::cerr << "round-trip: " << back->loudspeakers.size() << " speakers, expected "
                << res->loudspeakers.size() << "\n";
      return 1;
    }
    for(std::size_t i = 0; i < res->loudspeakers.size(); i++)
    {
      if(back->loudspeakers[i].name != res->loudspeakers[i].name)
      {
        std::cerr << "round-trip: speaker " << i << " is named '"
                  << res->loudspeakers[i].name << "' but came back as '"
                  << back->loudspeakers[i].name << "'\n";
        return 1;
      }
    }
  }

  return 0;
}