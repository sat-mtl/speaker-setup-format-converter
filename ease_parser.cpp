#pragma once

#include "ease_parser.hpp"

#include "ease_model.hpp"

#include <boost/regex.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/core/parse.hpp>
namespace x3 = boost::spirit::x3;

//////////////////////////////
// Parser
//////////////////////////////
// clang-format off

namespace spatparse::ease
{
x3::rule<class file_header_parser, file_header_t> const file_header_parser
    = "file_header_parser";

const auto text_character = ((x3::print - "\"") | x3::char_(" "));
const auto file_header_parser_def 
    = "\"FileType\",\"" >> +text_character >> '"'
      >> "\"Format\"," >> x3::double_
      >> "\"LengthUnit\",\"" >> +text_character >> '"';

x3::rule<class loudspeaker_parser, loudspeaker_t> const loudspeaker_parser
    = "loudspeaker_parser";

const auto loudspeaker_parser_def
    = "\"Label\",\"" >> +text_character >> x3::lit("\"")
      >> "\"Position\"," >> x3::double_ >> ',' >> x3::double_ >> ',' >> x3::double_ 
      >> "\"Ver/Hor/Rot\"," >> x3::double_ >> ',' >> x3::double_ >> ',' >> x3::double_ 
      >> "\"Speaker\",\"" >> +text_character >> x3::lit("\"") 
      >> "\"Delay/Align\"," >> x3::int_ >> ',' >> x3::int_ 
      >> "\"dB 1m\"," >> *(x3::int_ % ',') 
      >> "\"Watts\"," >> x3::int_
      >> "\"Phase\"," >> x3::int_;

BOOST_SPIRIT_DEFINE(file_header_parser, loudspeaker_parser);
// clang-format on

std::optional<file> parse(std::string_view input)
{
  file res;

  std::string cleaned{input};

  // Remove the comments
  const boost::regex rx(";([[:print:]]| )*\n");
  cleaned = boost::regex_replace(cleaned, rx, "");

  auto begin = cleaned.begin();
  auto end = cleaned.end();
  bool success = boost::spirit::x3::phrase_parse(
      begin, end, file_header_parser >> *loudspeaker_parser, x3::ascii::space, res);
  if(!success)
  {
    return std::nullopt;
  }
  return res;
}
}
