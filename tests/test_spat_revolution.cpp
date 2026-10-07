#include "spat_revolution_parser.hpp"
#include "converter.hpp"

#include <clocale>
#include <iostream>
#include <fstream>

#include <algorithm>
#include <array>
#include <cmath>

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
    // Positions have to survive being written out. The writer used {:.1f}, so an angle
    // was quantised to 0.1 degree -- radius*1.7e-3 of displacement, half a metre on a
    // 300 m layout -- and Distance to 10 cm. Flux's own files carry up to 14 decimals, so
    // nothing about the format required that.
    {
      spatparse::spat_revolution::file f;
      spatparse::spat_revolution::configuration conf;
      for(auto [az, el, d] : {std::array{12.345678, 7.654321, 2.718282},
                              std::array{-123.456789, -45.678901, 314.159265},
                              std::array{0.049999, 0.049999, 0.949999}})
      {
        spatparse::spat_revolution::channel ch;
        ch.name = "spk";
        ch.azimuth = az;
        ch.elevation = el;
        ch.distance = d;
        conf.channels.push_back(ch);
      }
      f.configurations.push_back(conf);

      const auto back = spatparse::spat_revolution::parse(
          spatparse::spat_revolution::to_string(f));
      if(!back || back->configurations.empty())
      {
        std::cerr << "precision: the file we just wrote does not parse\n";
        return 1;
      }
      const auto& in = f.configurations[0].channels;
      const auto& out = back->configurations[0].channels;
      if(in.size() != out.size())
      {
        std::cerr << "precision: " << out.size() << " channels, expected " << in.size()
                  << "\n";
        return 1;
      }
      for(std::size_t i = 0; i < in.size(); i++)
      {
        const double e = std::max({std::abs(in[i].azimuth - out[i].azimuth),
                                   std::abs(in[i].elevation - out[i].elevation),
                                   std::abs(in[i].distance - out[i].distance)});
        if(e > 1e-5)
        {
          std::cerr << "precision: channel " << i << " moved by " << e
                    << " through the writer\n";
          return 1;
        }
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
