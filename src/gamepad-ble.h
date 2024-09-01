#pragma once

/* gamepad-ble library by Julien Vanier <jvanier@gmail.com>
 */

// This will load the definition for common Particle variable types
#include "Particle.h"

class Gamepad
{
public:
  /**
   * Function to parse the data to buttons
   */
  using ParseDataFunction = std::function<void(Gamepad &, const uint8_t *, size_t)>;

  /**
   * Constructor
   */
  Gamepad();

  /**
   * Constructor with a custom parse function
   */
  Gamepad(ParseDataFunction customParseFunction);

  /**
   * Call from setup()
   */
  void begin();

  /**
   * Call from loop()
   */
  void process();

  /**
   * True if connected and paired to a gamepad and receiving data.
   */
  bool valid();

  /**
   * Device name
   */
  String name;

  /**
   * Raw data
   */
  Vector<uint8_t> data;

  /* Axes and buttons */
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  uint16_t leftTrigger;
  uint16_t rightTrigger;
  uint8_t dpad;
  bool a;
  bool b;
  bool x;
  bool y;
  bool leftBumper;
  bool rightBumper;
  bool leftStick;
  bool rightStick;
  bool xbox;
  bool view;
  bool share;
  bool menu;

private:
  void connect();
  bool dataReceived;
  std::optional<ParseDataFunction> parseFunction;
  std::optional<ParseDataFunction> customParseFunction;

  BlePeerDevice peer;
  BleCharacteristic inputReportCharacteristic;
};
