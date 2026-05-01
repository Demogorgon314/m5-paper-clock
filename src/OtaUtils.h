#pragma once

#include <Arduino.h>

namespace ota {

String BytesToHex(const uint8_t* bytes, size_t length);
String NormalizedSha256(String value);

}  // namespace ota
