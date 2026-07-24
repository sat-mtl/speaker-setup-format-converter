#include "speakerview_parser.hpp"

#include <clocale>
#include <cmath>
#include <iostream>

static constexpr char sample_speakerview[] = {
#embed "speakerview_sample.xml"
};

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  auto res = spatparse::spatgris::parse(
      std::string_view(sample_speakerview, std::ssize(sample_speakerview)));
  if(!res)
  {
    std::cerr << "Parsing failed\n";
    return 1;
  }

  if(res->mode != "Dome")
  {
    std::cerr << "Unexpected mode: " << res->mode << "\n";
    return 1;
  }

  auto speakers = spatparse::spatgris::all_speakers(*res);
  if(speakers.size() != 18)
  {
    std::cerr << "Expected 18 speakers, got " << speakers.size() << "\n";
    return 1;
  }

  for(auto* sp : speakers)
  {
    std::cerr << sp->name << " => " << sp->x << ", " << sp->y << ", " << sp->z << "\n";
  }

  // First speaker of the sample, as written in speakerview_sample.xml
  const auto& first = *speakers.front();
  if(std::abs(first.x - -0.3826835751533508) > 1e-9
     || std::abs(first.y - 0.9238796830177307) > 1e-9)
  {
    std::cerr << "Unexpected position for the first speaker\n";
    return 1;
  }
}
