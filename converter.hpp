#pragma once

#include "aiira_model.hpp"
#include "csv_model.hpp"
#include "ease_model.hpp"
#include "fourdsound_model.hpp"
#include "generic_model.hpp"
#include "spat_model.hpp"
#include "spat_revolution_model.hpp"
#include "speakerview_model.hpp"

#include <boost/mp11.hpp>
namespace spatparse
{
using all_formats = boost::mp11::mp_list<
    spatgris::file, ease::file, spat::file, aiira::file, csv::file,
    fourdsound::file, spat_revolution::file>;

void convert(const spatgris::file& input, unified::loudspeaker_configuration& output);
void convert(const ease::file& input, unified::loudspeaker_configuration& output);
void convert(const spat::file& input, unified::loudspeaker_configuration& output);
void convert(const aiira::file& input, unified::loudspeaker_configuration& output);
void convert(const csv::file& input, unified::loudspeaker_configuration& output);
void convert(const fourdsound::file& input, unified::loudspeaker_configuration& output);
void convert(
    const spat_revolution::file& input, unified::loudspeaker_configuration& output);

void convert(const unified::loudspeaker_configuration& input, spatgris::file& output);
void convert(const unified::loudspeaker_configuration& input, ease::file& output);
void convert(const unified::loudspeaker_configuration& input, spat::file& output);
void convert(const unified::loudspeaker_configuration& input, aiira::file& output);
void convert(const unified::loudspeaker_configuration& input, csv::file& output);
void convert(const unified::loudspeaker_configuration& input, fourdsound::file& output);
void convert(
    const unified::loudspeaker_configuration& input, spat_revolution::file& output);

template <typename From, typename To>
  requires(
      !(std::is_same_v<From, unified::loudspeaker_configuration>
        || std::is_same_v<To, unified::loudspeaker_configuration>))
void convert(const From& from, To& to)
{
  unified::loudspeaker_configuration mid;
  convert(from, mid);
  convert(mid, to);
}

template <typename To>
To convert(const auto& from)
{
  To to;
  convert(from, to);
  return to;
}
}
