#pragma once

#include "aiira_model.hpp"
#include "csv_model.hpp"
#include "ease_model.hpp"
#include "spat_model.hpp"
#include "speakerview_model.hpp"

namespace spatparse
{
speakerview::file to_speakerview(ease::file);
speakerview::file to_speakerview(spat::file);
speakerview::file to_speakerview(aiira::file);
speakerview::file to_speakerview(csv::file);
}
