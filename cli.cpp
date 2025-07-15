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

static const std::set<std::string> supported_formats{"xld",  "ease",  "csv",
                                                     "spat_ircam", "iem", "spatgris"};

bool process(const cli_options& opts)
{
  std::ifstream in_file(opts.filename);
  if(!in_file.is_open())
    throw std::runtime_error("Cannot open file!");

  auto in_format_it = supported_formats.find(opts.in_format);
  if(in_format_it == supported_formats.end())
    throw std::runtime_error("Wrong input format!");

  auto out_format_it = supported_formats.find(opts.out_format);
  if(out_format_it == supported_formats.end())
    throw std::runtime_error("Wrong output format!");

  // Read file
  const auto size = std::filesystem::file_size(opts.filename);
  if(size >= 1024 * 1024 * 1024)
    throw std::runtime_error("File unsupported (weirdly large size > 1GB)");

  std::string bytes(size, '\0');
  in_file.read(&bytes[0], size);

  spatparse::speakerview::fixup_options out_opts{
      .normalize = opts.normalize, .recenter = opts.recenter};

  // 1. Parse input into the generic format
  spatparse::unified::loudspeaker_configuration parsed;
  if(opts.in_format == "ease" || opts.in_format == "xld")
  {
    if(auto in = spatparse::ease::parse(bytes))
      spatparse::convert(*in, parsed);
  }
  else if(opts.in_format == "csv")
  {
    if(auto in = spatparse::csv::parse(bytes))
      spatparse::convert(*in, parsed);
  }
  else if(opts.in_format == "spat_ircam")
  {
    if(auto in = spatparse::spat::parse(bytes))
      spatparse::convert(*in, parsed);
  }
  else if(opts.in_format == "iem")
  {
    if(auto in = spatparse::aiira::parse(bytes))
      spatparse::convert(*in, parsed);
  }
  else if(opts.in_format == "spatgris")
  {
    if(auto in = spatparse::speakerview::parse(bytes))
      spatparse::convert(*in, parsed);
  }

  if(parsed.loudspeakers.empty())
    throw std::runtime_error(std::format("Could not parse {} input", opts.in_format));

  // 2. Convert generic format into desired output format

  if(opts.out_format == "ease" || opts.out_format == "xld")
  {
    spatparse::ease::file res;
    spatparse::convert(parsed, res);
    // output(res);
  }
  else if(opts.out_format == "csv")
  {
    spatparse::csv::file res;
    spatparse::convert(parsed, res);
  }
  else if(opts.out_format == "spat_ircam")
  {
    spatparse::spat::file res;
    spatparse::convert(parsed, res);
  }
  else if(opts.out_format == "iem")
  {
    spatparse::aiira::file res;
    spatparse::convert(parsed, res);
  }
  else if(opts.out_format == "spatgris")
  {
    spatparse::speakerview::file res;
    spatparse::convert(parsed, res);
    spatparse::speakerview::fixup(res, out_opts);
    std::cout << spatparse::speakerview::to_string(res) << std::endl;
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
      "my_file.xld");
  parser.helpParams.width = 120;
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::CompletionFlag completion(parser, {"complete"});

  args::Positional<std::string> filename(
      parser, "filename", "File to open", args::Options::Required);
  args::ValueFlag<std::string> in_format(
      parser, "input file format", "One of xld, ease, csv, spat, aiira", {"in-format"});
  args::ValueFlag<std::string> out_format(
      parser, "output file format", "One of xld, ease, csv, spat, aiira",
      {"out-format"});

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
