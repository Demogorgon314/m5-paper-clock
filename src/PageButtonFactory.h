#pragma once

#include <Arduino.h>

#include <vector>

#include "UiTypes.h"

namespace pagebuttons {

void BuildSettingsButtons(std::vector<UiButton>& buttons);
void BuildWifiButtons(std::vector<UiButton>& buttons, int network_count,
                      int page_index, int page_size);
void BuildPasswordButtons(std::vector<UiButton>& buttons,
                          const std::vector<String>& keys,
                          bool password_visible, bool keyboard_symbols);

}  // namespace pagebuttons
