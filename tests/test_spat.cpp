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
}
