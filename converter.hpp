#pragma once

#include "aiira_model.hpp"
#include "csv_model.hpp"
#include "ease_model.hpp"
#include "fourdsound_model.hpp"
#include "generic_model.hpp"
#include "spat_model.hpp"
#include "speakerview_model.hpp"

namespace spatparse
{
void convert(const speakerview::file& input, unified::loudspeaker_configuration& output);
void convert(const ease::file& input, unified::loudspeaker_configuration& output);
void convert(const spat::file& input, unified::loudspeaker_configuration& output);
void convert(const aiira::file& input, unified::loudspeaker_configuration& output);
void convert(const csv::file& input, unified::loudspeaker_configuration& output);
void convert(const fourdsound::file& input, unified::loudspeaker_configuration& output);

void convert(const unified::loudspeaker_configuration& input, speakerview::file& output);
void convert(const unified::loudspeaker_configuration& input, ease::file& output);
void convert(const unified::loudspeaker_configuration& input, spat::file& output);
void convert(const unified::loudspeaker_configuration& input, aiira::file& output);
void convert(const unified::loudspeaker_configuration& input, csv::file& output);
void convert(const unified::loudspeaker_configuration& input, fourdsound::file& output);

speakerview::file to_speakerview(ease::file);
speakerview::file to_speakerview(spat::file);
speakerview::file to_speakerview(aiira::file);
speakerview::file to_speakerview(csv::file);
speakerview::file to_speakerview(fourdsound::file);
}
