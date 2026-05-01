#pragma once

#include <Arduino.h>

#include "AppSettingsTypes.h"
#include "LayoutDocumentCodec.h"

namespace layoutsettings {

struct ApplyDraftResult {
    bool market_changed = false;
    bool comfort_changed = false;
    bool date_changed = false;
};

ApplyDraftResult ApplyLayoutDraft(AppSettings& settings,
                                   const layoutdoc::LayoutApplyDraft& draft);

}  // namespace layoutsettings
