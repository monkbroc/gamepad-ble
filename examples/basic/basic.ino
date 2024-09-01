#include "gamepad-ble.h"

SerialLogHandler logHandler(LOG_LEVEL_INFO);

Gamepad gamepad;

void setup() {
  gamepad.begin();
}

void loop() {
  gamepad.process();

  if (gamepad.valid()) {
    Log.info("x1=%5d y1=%5d %s %s %s %s", gamepad.x1, gamepad.y1,
      gamepad.a ? "a" : " ", gamepad.b ? "b" : " ",
      gamepad.x ? "x" : " ", gamepad.y ? "y" : " ");
  }
}