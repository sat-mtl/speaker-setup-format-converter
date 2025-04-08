#include "parser.hpp"

#include <boost/regex.hpp>

#include <clocale>
#include <iostream>
#include <string>
#include <vector>

//////////////////////////////
// Test
//////////////////////////////
int main()
{
  std::string input = R"(
; EASE Focus 3 - Soci?t? des arts technologiques - David Ledoux
;
; Exported on 2025-04-08 15:01
;
;==============================================================
"FileType","Loudspeakers"
"Format",4.1
"LengthUnit","m"
;
"Label","AD-S.SAT"
"Position",22.12,17.7,2.2
"Ver/Hor/Rot",-30,0,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 1"
"Position",23.12,17.7,2.2
"Ver/Hor/Rot",-30,0,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 10"
"Position",22.6,16.7,2.2
"Ver/Hor/Rot",-90,0,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 11"
"Position",22.6,15.5,2.2
"Ver/Hor/Rot",-90,180,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 2"
"Position",21.5,15.12,2.2
"Ver/Hor/Rot",-30,90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 3"
"Position",21.5,17.15,2.2
"Ver/Hor/Rot",-30,90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 4"
"Position",23.7,17.15,2.2
"Ver/Hor/Rot",-30,-90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 5"
"Position",23.7,15.13,2.2
"Ver/Hor/Rot",-30,-90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 6"
"Position",22.12,14.5,2.2
"Ver/Hor/Rot",-30,180,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 7"
"Position",23.12,14.5,2.2
"Ver/Hor/Rot",-30,180,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 8"
"Position",21.5,16.14,2.2
"Ver/Hor/Rot",-30,90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SAT 9"
"Position",23.7,16.15,2.2
"Ver/Hor/Rot",-30,-90,0
"Speaker","AD-S.SAT"
"Delay/Align",0,0
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
"Label","AD-S.SUB 1"
"Position",22.7,17.93,2.5
"Ver/Hor/Rot",0,-90,0
"Speaker","AD-S.SUB"
"Delay/Align",0,0
"dB 1m",-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2
"Watts",0
"Phase",0
;
"Label","AD-S.SUB 2"
"Position",22.6,12.8,2.5
"Ver/Hor/Rot",0,-90,0
"Speaker","AD-S.SUB"
"Delay/Align",13,400
"dB 1m",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
"Watts",0
"Phase",0
;
)";
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  // Remove the comments
  const boost::regex rx(";([[:print:]]| )*\n");
  input = boost::regex_replace(input, rx, "");

  // Parse
  struct file file;

  auto begin = input.begin();
  auto end = input.end();
  bool success = boost::spirit::x3::phrase_parse(
      begin, end, file_header_parser >> *loudspeaker_parser, x3::ascii::space, file);

  // Print
  std::cout << file.header.file_type << "\n"
            << file.header.format << "\n"
            << file.header.length_unit << "\n";

  for(int i = 0; i < file.loudspeakers.size(); i++)
  {
    auto& l = file.loudspeakers[i];
    std::cout << i << ":\n"
              << l.label << "\n"
              << l.x << " " << l.y << " " << l.z << "\n"
              << l.ver << " " << l.hor << " " << l.rot << "\n"
              << l.speaker << "\n"
              << l.delay << "\n"
              << l.align << "\n";
    std::cout << "[";
    for(auto db : l.db_1m)
      std::cout << db << ",";

    std::cout << "]\n";
    std::cout << l.phase << "\n" << l.watts << "\n\n";
  }

  return 0;
}
