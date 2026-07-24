// Files exported by Windows tools (EASE, Panoramix, Excel...) use CRLF.
// The samples are stored with LF, so each one is converted in memory here and
// has to parse to the same thing as its LF original.
#include "aiira_parser.hpp"
#include "csv_parser.hpp"
#include "ease_parser.hpp"
#include "spat_parser.hpp"
#include "speakerview_parser.hpp"

#include <clocale>
#include <iostream>
#include <string>

static constexpr char sample_ease[] = {
#embed "ease_sample.ease"
};

static constexpr char sample_csv[] = {
#embed "csv_sample.csv"
};

static constexpr char sample_spat[] = {
#embed "spat_panoramix_sample.rtf"
};

static constexpr char sample_speakerview[] = {
#embed "speakerview_sample.xml"
};

static constexpr char sample_aiira[] = {
#embed "aiira_sample.json"
};

static std::string to_crlf(std::string_view input)
{
  std::string out;
  out.reserve(input.size() + input.size() / 16);
  for(char c : input)
  {
    if(c == '\n')
      out += '\r';
    out += c;
  }
  return out;
}

static int failures = 0;

template <typename Parsed>
static void check(const char* what, const Parsed& lf, const Parsed& crlf)
{
  if(!lf)
  {
    std::cerr << what << ": the LF sample does not parse, the test is broken\n";
    failures++;
    return;
  }

  if(!crlf)
  {
    std::cerr << what << ": parsing failed on CRLF input\n";
    failures++;
    return;
  }

  std::cerr << what << ": ok\n";
}

int main()
{
  // Make sure numbers are parsed with dots
  setlocale(LC_ALL, "C");

  const std::string_view ease{sample_ease, std::size(sample_ease)};
  const std::string_view csv{sample_csv, std::size(sample_csv)};
  const std::string_view spat{sample_spat, std::size(sample_spat)};
  const std::string_view speakerview{sample_speakerview, std::size(sample_speakerview)};
  const std::string_view aiira{sample_aiira, std::size(sample_aiira)};

  const auto ease_crlf = to_crlf(ease);
  const auto csv_crlf = to_crlf(csv);
  const auto spat_crlf = to_crlf(spat);
  const auto speakerview_crlf = to_crlf(speakerview);
  const auto aiira_crlf = to_crlf(aiira);

  const auto ease_lf_res = spatparse::ease::parse(ease);
  const auto ease_crlf_res = spatparse::ease::parse(ease_crlf);
  check("ease", ease_lf_res, ease_crlf_res);

  const auto csv_lf_res = spatparse::csv::parse(csv);
  const auto csv_crlf_res = spatparse::csv::parse(csv_crlf);
  check("csv", csv_lf_res, csv_crlf_res);

  const auto spat_lf_res = spatparse::spat::parse(spat);
  const auto spat_crlf_res = spatparse::spat::parse(spat_crlf);
  check("spat", spat_lf_res, spat_crlf_res);

  const auto sv_lf_res = spatparse::spatgris::parse(speakerview);
  const auto sv_crlf_res = spatparse::spatgris::parse(speakerview_crlf);
  check("speakerview", sv_lf_res, sv_crlf_res);

  const auto aiira_lf_res = spatparse::aiira::parse(aiira);
  const auto aiira_crlf_res = spatparse::aiira::parse(aiira_crlf);
  check("aiira", aiira_lf_res, aiira_crlf_res);

  // Same number of speakers, and the same values, whatever the line endings
  if(ease_lf_res && ease_crlf_res
     && ease_lf_res->loudspeakers.size() != ease_crlf_res->loudspeakers.size())
  {
    std::cerr << "ease: " << ease_lf_res->loudspeakers.size() << " speakers in LF, "
              << ease_crlf_res->loudspeakers.size() << " in CRLF\n";
    failures++;
  }

  if(csv_lf_res && csv_crlf_res
     && csv_lf_res->speakers.size() != csv_crlf_res->speakers.size())
  {
    std::cerr << "csv: " << csv_lf_res->speakers.size() << " speakers in LF, "
              << csv_crlf_res->speakers.size() << " in CRLF\n";
    failures++;
  }

  if(spat_lf_res && spat_crlf_res)
  {
    if(spat_lf_res->loudspeakers.size() != spat_crlf_res->loudspeakers.size())
    {
      std::cerr << "spat: " << spat_lf_res->loudspeakers.size() << " speakers in LF, "
                << spat_crlf_res->loudspeakers.size() << " in CRLF\n";
      failures++;
    }
    else
    {
      for(std::size_t i = 0; i < spat_lf_res->loudspeakers.size(); i++)
      {
        // The name is the part that a leftover '\r' used to silently drop
        if(spat_lf_res->loudspeakers[i].name != spat_crlf_res->loudspeakers[i].name)
        {
          std::cerr << "spat: speaker " << i << " is named '"
                    << spat_lf_res->loudspeakers[i].name << "' in LF but '"
                    << spat_crlf_res->loudspeakers[i].name << "' in CRLF\n";
          failures++;
        }
      }
    }
  }

  // A speaker index far past anything reasonable must be rejected, not
  // turned into an allocation of that size
  const auto hostile = spatparse::spat::parse(
      "/speaker/number 2\n"
      "/speaker/2000000000/name \"boom\"\n");
  if(hostile && hostile->loudspeakers.size() > 65535)
  {
    std::cerr << "spat: a bogus speaker index allocated "
              << hostile->loudspeakers.size() << " speakers\n";
    failures++;
  }

  return failures == 0 ? 0 : 1;
}
