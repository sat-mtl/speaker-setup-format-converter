#include "aiira_parser.hpp"
#include "converter.hpp"
#include "csv_parser.hpp"
#include "ease_parser.hpp"
#include "spat_parser.hpp"
#include "speakerview_parser.hpp"

#include <clocale>
#include <iostream>

static constexpr char sample_aiira[] = {
#embed "aiira_sample.json"
};

static constexpr char sample_csv[] = {
#embed "csv_sample.csv"
};

static constexpr char sample_ease[] = {
#embed "ease_sample.ease"
};

static constexpr char sample_rtf[] = {
#embed "spat_panoramix_sample.rtf"
};

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");
  {
    auto in
        = spatparse::spat::parse(std::string_view(sample_rtf, std::ssize(sample_rtf)));
    auto converted = spatparse::to_speakerview(*in);
    spatparse::speakerview::fixup(converted);
    auto string = spatparse::speakerview::to_string(converted);
    std::cerr << string << std::endl;
  }

  {
    auto in
        = spatparse::csv::parse(std::string_view(sample_csv, std::ssize(sample_csv)));
    auto converted = spatparse::to_speakerview(*in);
    spatparse::speakerview::fixup(converted);
    auto string = spatparse::speakerview::to_string(converted);
    std::cerr << string << std::endl;
  }

  {
    auto in = spatparse::aiira::parse(
        std::string_view(sample_aiira, std::ssize(sample_aiira)));
    auto converted = spatparse::to_speakerview(*in);
    spatparse::speakerview::fixup(converted);
    auto string = spatparse::speakerview::to_string(converted);
    std::cerr << string << std::endl;
  }

  {
    auto in
        = spatparse::ease::parse(std::string_view(sample_ease, std::ssize(sample_ease)));
    auto converted = spatparse::to_speakerview(*in);
    spatparse::speakerview::fixup(converted);
    auto string = spatparse::speakerview::to_string(converted);
    std::cerr << string << std::endl;
  }
}
