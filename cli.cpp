#include "aiira_parser.hpp"
#include "args.hxx"
#include "converter.hpp"
#include "csv_parser.hpp"
#include "ease_parser.hpp"
#include "spat_parser.hpp"
#include "speakerview_parser.hpp"

#include <clocale>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
struct cli_options
{
  std::string filename;
  std::string in_format;
  std::string out_format;
  std::optional<double> normalize{};
  bool recenter{};
};

bool process(const cli_options& opts)
{
  std::ifstream in_file(opts.filename);
  if(!in_file.is_open())
    throw std::runtime_error("Cannot open file!");

  std::set<std::string> in_formats{"ease", "csv", "spat", "aiira"};
  auto it = in_formats.find(opts.in_format);
  if(it == in_formats.end())
    throw std::runtime_error("Wrong input format!");

  // FIXME out format when we have another than speakerview

  // Read file
  auto size = std::filesystem::file_size(opts.filename);
  if(size >= 1024 * 1024 * 1024)
    throw std::runtime_error("File unsupported (weirdly large size > 1GB)");

  std::string bytes(size, '\0');
  in_file.read(&bytes[0], size);

  spatparse::speakerview::fixup_options out_opts{
      .normalize = opts.normalize, .recenter = opts.recenter};

  auto output = [&](spatparse::speakerview::file&& converted) {
    spatparse::speakerview::fixup(converted, out_opts);
    auto string = spatparse::speakerview::to_string(converted);
    std::cout << string << std::endl;
  };
  if(opts.in_format == "ease")
  {
    if(auto in = spatparse::ease::parse(bytes))
      output(spatparse::to_speakerview(*in));
    else
      throw std::runtime_error("Could not parse input");
  }
  else if(opts.in_format == "csv")
  {
    if(auto in = spatparse::csv::parse(bytes))
      output(spatparse::to_speakerview(*in));
    else
      throw std::runtime_error("Could not parse input");
  }
  else if(opts.in_format == "spat")
  {
    if(auto in = spatparse::spat::parse(bytes))
      output(spatparse::to_speakerview(*in));
    else
      throw std::runtime_error("Could not parse input");
  }
  else if(opts.in_format == "aiira")
  {
    if(auto in = spatparse::aiira::parse(bytes))
      output(spatparse::to_speakerview(*in));
    else
      throw std::runtime_error("Could not parse input");
  }
  return true;
}

int main(int argc, char** argv)
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  args::ArgumentParser parser(
      "spartparse CLI",
      "A tool for converting speaker position file formats.\n"
      "Example: ./spatparsecli --normalize --in-format=ease --out-format=speakerview "
      "my_file.ease");
  parser.helpParams.width = 120;
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::CompletionFlag completion(parser, {"complete"});

  args::Positional<std::string> filename(
      parser, "filename", "File to open", args::Options::Required);
  args::ValueFlag<std::string> in_format(
      parser, "input file format", "One of ease, csv, spat, aiira", {"in-format"});
  args::ValueFlag<std::string> out_format(
      parser, "output file format", "speakerview", {"out-format"});

  args::ValueFlag<double> normalize(
      parser, "normalize", "Rescale all distances to this ratio", {"normalize"}, -1e99);
  args::Flag center(parser, "center", "Center all the positions on (0,0)", {"center"});
  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch(const args::Completion& e)
  {
    std::cout << e.what();
    return 0;
  }
  catch(const args::Help&)
  {
    std::cout << parser;
    return 0;
  }
  catch(const args::ParseError& e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  cli_options opts;
  opts.filename = filename.Get();
  opts.in_format = in_format.Get();
  opts.out_format = out_format.Get();
  opts.recenter = center.Get();

  opts.normalize = normalize.Get();
  if(opts.normalize == -1e99)
    opts.normalize.reset();

  try
  {
    if(process(opts))
    {
      return 0;
    }
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  std::cout << parser;

  return 1;
}
