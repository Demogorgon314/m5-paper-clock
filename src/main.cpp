#include "ClockApp.h"

ClockApp app;

void setup() {
    app.begin();
}

void loop() {
    app.loop();
    delay(20);
}
