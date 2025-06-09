#include "aiira_parser.hpp"

#include <clocale>
#include <iostream>

// The sample JSON data provided in the problem description
const auto sample_json = R"({
  "Name": "All-Round Ambisonic decoder (AllRAD) and loudspeaker layout",
  "Description": "This configuration file was created with the IEM AllRADecoder v0.9.4 plug-in. 26 Jun 2024 8:00:40pm",
  "LoudspeakerLayout": {
    "Name": "A loudspeaker layout",
    "Loudspeakers": [
      { "Azimuth": 60.0, "Elevation": 50.0, "Radius": 1.0, "IsImaginary": false, "Channel": 1, "Gain": 1.0 },
      { "Azimuth": 180.0, "Elevation": 50.0, "Radius": 1.0, "IsImaginary": false, "Channel": 2, "Gain": 1.0 },
      { "Azimuth": -60.0, "Elevation": 50.0, "Radius": 1.0, "IsImaginary": false, "Channel": 3, "Gain": 1.0 },
      { "Azimuth": 0.0, "Elevation": 15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 4, "Gain": 1.0 },
      { "Azimuth": 60.0, "Elevation": -15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 5, "Gain": 1.0 },
      { "Azimuth": 120.0, "Elevation": 15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 6, "Gain": 1.0 },
      { "Azimuth": 180.0, "Elevation": -15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 7, "Gain": 1.0 },
      { "Azimuth": -120.0, "Elevation": 15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 8, "Gain": 1.0 },
      { "Azimuth": -60.0, "Elevation": -15.0, "Radius": 1.0, "IsImaginary": false, "Channel": 9, "Gain": 1.0 },
      { "Azimuth": 120.0, "Elevation": -50.0, "Radius": 1.0, "IsImaginary": false, "Channel": 10, "Gain": 1.0 },
      { "Azimuth": -120.0, "Elevation": -50.0, "Radius": 1.0, "IsImaginary": false, "Channel": 11, "Gain": 1.0 },
      { "Azimuth": 0.0, "Elevation": -50.0, "Radius": 0.1400000005960464, "IsImaginary": false, "Channel": 12, "Gain": 1.0 }
    ]
  }
})";

// The sample JSON data provided in the problem description
static constexpr char sample_json2[] = {
#embed "aiira_sample.json"
};

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");
  try
  {
    // Parse the file using our dedicated function
    auto maybe_file = spatparse::aiira::parse(
        std::string_view(sample_json2, std::ssize(sample_json2)));
    if(!maybe_file)
    {
      std::cerr << "Parse error\n";
      return 1;
    }
    auto& file = *maybe_file;

    // Print some of the parsed data to verify correctness
    std::cout << "Successfully parsed file." << std::endl;
    std::cout << "--- Header ---" << std::endl;
    std::cout << "Name: " << file.header.name << std::endl;
    std::cout << "Description: " << file.header.description << std::endl;

    std::cout << "\n--- Layout ---" << std::endl;
    std::cout << "Name: " << file.layout.name << std::endl;
    std::cout << "Loudspeaker count: " << file.layout.loudspeakers.size() << std::endl;

    // Verify the data from the first and last loudspeaker
    if(!file.layout.loudspeakers.empty())
    {
      const auto& first_speaker = file.layout.loudspeakers.front();
      const auto& last_speaker = file.layout.loudspeakers.back();

      std::cout << "\n--- First Loudspeaker (Channel " << first_speaker.channel
                << ") ---" << std::endl;
      std::cout << "Azimuth: " << first_speaker.azimuth
                << ", Elevation: " << first_speaker.elevation << std::endl;

      std::cout << "\n--- Last Loudspeaker (Channel " << last_speaker.channel << ") ---"
                << std::endl;
      std::cout << "Azimuth: " << last_speaker.azimuth
                << ", Elevation: " << last_speaker.elevation << std::endl;
      std::cout << "Radius: " << last_speaker.radius << std::endl;
    }

    return 0;
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
