#include "spat_revolution_parser.hpp"
#include "converter.hpp"

#include <clocale>
#include <iostream>
#include <fstream>

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");
  
  try
  {
    // Read the test file
    std::ifstream file("spat_revolution.ioconfig");
    if(!file.is_open())
    {
      std::cerr << "Could not open test file 'spat_revolution.ioconfig'\n";
      return 1;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    // Parse the file
    auto maybe_file = spatparse::spat_revolution::parse(content);
    if(!maybe_file)
    {
      std::cerr << "Parse error\n";
      return 1;
    }
    auto& parsed_file = *maybe_file;
    
    // Print parsed data
    std::cout << "Successfully parsed file." << std::endl;
    std::cout << "Number of configurations: " << parsed_file.configurations.size() << std::endl;
    
    for(size_t i = 0; i < parsed_file.configurations.size(); ++i)
    {
      const auto& config = parsed_file.configurations[i];
      std::cout << "\n--- Configuration " << (i + 1) << " ---" << std::endl;
      std::cout << "Name: " << config.name << std::endl;
      std::cout << "UID: " << config.uid << std::endl;
      std::cout << "Number of channels: " << config.channels.size() << std::endl;
      std::cout << "Stream type: " << config.stream_type << std::endl;
      std::cout << "Dimension: " << config.dimension << std::endl;
      
      if(!config.channels.empty())
      {
        const auto& first = config.channels.front();
        const auto& last = config.channels.back();
        
        std::cout << "\nFirst channel:" << std::endl;
        std::cout << "  Name: " << first.name << std::endl;
        std::cout << "  Azimuth: " << first.azimuth << "°" << std::endl;
        std::cout << "  Elevation: " << first.elevation << "°" << std::endl;
        std::cout << "  Distance: " << first.distance << "m" << std::endl;
        
        std::cout << "\nLast channel:" << std::endl;
        std::cout << "  Name: " << last.name << std::endl;
        std::cout << "  Azimuth: " << last.azimuth << "°" << std::endl;
        std::cout << "  Elevation: " << last.elevation << "°" << std::endl;
        std::cout << "  Distance: " << last.distance << "m" << std::endl;
      }
    }
    return 0;
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
