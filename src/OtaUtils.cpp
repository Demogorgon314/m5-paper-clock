#include "OtaUtils.h"

namespace ota {

String BytesToHex(const uint8_t* bytes, size_t length) {
    static const char* hex = "0123456789abcdef";
    String output;
    output.reserve(length * 2);
    for (size_t index = 0; index < length; ++index) {
        output += hex[bytes[index] >> 4];
        output += hex[bytes[index] & 0x0f];
    }
    return output;
}

String NormalizedSha256(String value) {
    value.trim();
    value.toLowerCase();
    return value;
}

}  // namespace ota
