// Example usage for gamepad-ble library by Julien Vanier <jvanier@gmail.com>.

#include "gamepad-ble.h"

// Initialize objects from the lib
Gamepadble gamepadble;

void setup() {
    // Call functions on initialized library objects that require hardware
    gamepadble.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    gamepadble.process();
}
