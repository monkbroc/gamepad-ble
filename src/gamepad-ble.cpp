/* gamepad-ble library by Julien Vanier <jvanier@gmail.com>
 */

#include "gamepad-ble.h"

LOG_SOURCE_CATEGORY("app.gamepad-ble");

void xboxParseData(Gamepad &gamepad, const uint8_t *d, size_t len)
{
  if (len == 16)
  {
    gamepad.x1 = d[1] << 8 | d[0];
    gamepad.y1 = d[3] << 8 | d[2];
    gamepad.x2 = d[5] << 8 | d[4];
    gamepad.y2 = d[7] << 8 | d[6];
    gamepad.leftTrigger = d[9] << 8 | d[8];
    gamepad.rightTrigger = d[11] << 8 | d[10];
    gamepad.dpad = d[12];
    gamepad.a = d[13] & 0x01;
    gamepad.b = d[13] & 0x02;
    gamepad.x = d[13] & 0x08;
    gamepad.y = d[13] & 0x10;
    gamepad.leftBumper = d[13] & 0x40;
    gamepad.rightBumper = d[13] & 0x80;
    gamepad.leftStick = d[14] & 0x20;
    gamepad.rightStick = d[14] & 0x40;
    gamepad.xbox = d[14] & 0x10;
    gamepad.view = d[14] & 0x04;
    gamepad.share = d[15] & 0x01;
    gamepad.menu = d[14] & 0x08;
  }
  else
  {
    Log.info("Unexpected payload of %d bytes", len);
  }
}

BleUuid nameUuid(0x2A00);
BleUuid reportMapUuid(0x2A4B);
BleUuid reportUuid(0x2A4D);

/**
 * Constructor
 */
Gamepad::Gamepad() : name(),
                     data(),
                     x1(0),
                     y1(0),
                     x2(0),
                     y2(0),
                     leftTrigger(0),
                     rightTrigger(0),
                     dpad(0),
                     a(false),
                     b(false),
                     x(false),
                     y(false),
                     leftBumper(false),
                     rightBumper(false),
                     leftStick(false),
                     rightStick(false),
                     xbox(false),
                     view(false),
                     share(false),
                     menu(false),
                     dataReceived(false)
{
}

/**
 * Constructor with a custom parse function
 */
Gamepad::Gamepad(ParseDataFunction customParseFunction) : Gamepad()
{
  this->customParseFunction = customParseFunction;
}

/**
 * Call from setup()
 * - turns on BLE
 */
void Gamepad::begin()
{
  BLE.on();
  // Only scan for 1 second
  BLE.setScanTimeout(100);
}

/**
 * Call from loop()
 * - Scans BLE if not connected
 */
void Gamepad::process()
{
  if (!BLE.isPaired(peer))
  {
    connect();
  }
}

/**
 * True if connected and paired to a gamepad and receiving data.
 */
bool Gamepad::valid()
{
  return BLE.isPaired(peer) && dataReceived;
}

/**
 * Scan and connect
 */
void Gamepad::connect()
{
  Log.info("starting scan...");
  dataReceived = false;
  BleScanFilter filter;
  filter.appearance(BLE_SIG_APPEARANCE_HID_GAMEPAD);
  auto devices = BLE.scanWithFilter(filter);
  Log.info("%d gamepads found", devices.size());
  if (devices.size() == 0)
  {
    return;
  }

  BleAddress addr = devices.first().address();
  Log.info("Connecting to %s...", addr.toString().c_str());
  // BleConnectionParams params;
  // params.conn_sup_timeout = 100; // timeout BLE connection after 1 second
  peer = BLE.connect(addr);
  if (peer.connected())
  {
    name = "?";
    BleCharacteristic nameCharacteristic;
    if (peer.getCharacteristicByUUID(nameCharacteristic, nameUuid))
    {
      nameCharacteristic.getValue(name);
    }
    Log.info("successfully connected to %s", name.c_str());

    if (customParseFunction.has_value())
    {
      parseFunction = customParseFunction;
    }
    else if (name.startsWith("Xbox"))
    {
      parseFunction = xboxParseData;
    }
    else
    {
      parseFunction.reset();
    }

    BLE.startPairing(peer);
    while (BLE.isPairing(peer))
    {
      delay(100);
    }
    if (BLE.isPaired(peer))
    {
      Log.info("paired");
    }
    else
    {
      Log.info("pairing failed");
      peer.disconnect();
      return;
    }

    for (auto &ch : peer.characteristics())
    {
      if (ch.UUID() == reportMapUuid)
      {
        // read report map to exit pairing mode
        // don't bother parsing the report map as we support a hard-coded number of devices
        uint8_t buf[256] = {0};
        auto len = ch.getValue(buf, sizeof(buf));
        Log.info("report map len=%d", len);
      }

      if (ch.UUID() == reportUuid && (ch.properties() & BleCharacteristicProperty::NOTIFY))
      {
        inputReportCharacteristic = ch;
        inputReportCharacteristic.onDataReceived(
            [this](const uint8_t *data, size_t len, const BlePeerDevice &peer)
            {
              SINGLE_THREADED_SECTION();
              this->data.clear();
              for (size_t i = 0; i < len; i++)
              {
                this->data.append(data[i]);
              }
              if (this->parseFunction.has_value())
              {
                this->parseFunction.value()(*this, data, len);
              }
              dataReceived = true;
            });
      }
    }
  }
}