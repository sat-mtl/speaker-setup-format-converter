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

  // fixup() snaps near-zero coordinates to zero, and used to do it with a one-sided
  // comparison -- which moved every speaker with a negative coordinate onto the origin.
  // Six speakers on the unit axes must come out unchanged.
  spatparse::spatgris::file axes;
  for(auto [x, y, z] : {std::array{1., 0., 0.}, std::array{-1., 0., 0.},
                        std::array{0., 1., 0.}, std::array{0., -1., 0.},
                        std::array{0., 0., 1.}, std::array{0., 0., -1.}})
    axes.children.push_back(spatparse::spatgris::loudspeaker{
        .name = "axis", .x = x, .y = y, .z = z});

  spatparse::spatgris::fixup(axes);
  auto fixed = spatparse::spatgris::all_speakers(axes);
  if(fixed.size() != 6)
  {
    std::cerr << "fixup() lost speakers: " << fixed.size() << "\n";
    return 1;
  }
  int i = 0;
  for(auto [x, y, z] : {std::array{1., 0., 0.}, std::array{-1., 0., 0.},
                        std::array{0., 1., 0.}, std::array{0., -1., 0.},
                        std::array{0., 0., 1.}, std::array{0., 0., -1.}})
  {
    const auto& sp = *fixed[i];
    if(std::abs(sp.x - x) > 1e-12 || std::abs(sp.y - y) > 1e-12
       || std::abs(sp.z - z) > 1e-12)
    {
      std::cerr << "fixup() moved axis speaker " << i << ": expected " << x << "," << y
                << "," << z << " got " << sp.x << "," << sp.y << "," << sp.z << "\n";
      return 1;
    }
    ++i;
  }
}
