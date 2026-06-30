// ---------------------------------------------------------------------------
// GDDLink — the easiest way to talk to the Arduino Bluetooth (GDD) app.
//
// Covers every screen the app has, not just Switch I/O. You still read
// incoming bytes yourself with the normal Arduino pattern (if/while
// available() + read()) — this library turns the bytes you hand it into
// the right callback for whichever screen sent them:
//
//   #include <GDDLink.h>
//   GDDLink gdd(BT);                  // BT = any Stream: BluetoothSerial,
//                                      // HardwareSerial, SoftwareSerial...
//                                      // (NOT named "link" — that collides
//                                      // with a libc symbol on ESP32 boards)
//   void onGesture(const String &name) {
//     if (name == "FIST") digitalWrite(LED, HIGH);
//   }
//   void setup() {
//     BT.begin("GDD-ESP32");
//     gdd.onGesture(onGesture);       // Hand screen, trained/built-in poses
//   }
//   void loop() {
//     while (BT.available()) gdd.feed(BT.read());  // you read, gdd parses
//     gdd.poll();                                  // sends watch()ed values
//   }
//
// One callback per screen, registered in setup() — see examples/ for a
// short sketch per screen (Hand, Gamepad, Servo, Car, Tilt, Voice, Switch
// I/O). Switch I/O additionally supports the channel-name style shown in
// examples/SwitchIO (bindDigitalOut/bindAnalogOut/watch/onReceive), since
// that screen's "NAME:value" messages map naturally onto named channels.
// ---------------------------------------------------------------------------

#pragma once

#include <Arduino.h>

// How many distinct Switch I/O channel names one GDDLink can track, fixed
// at compile time — no heap allocation, so it's safe on small AVR boards
// (Uno/Nano only have 2 KB of RAM; a `new`'d array risks heap fragmentation
// there). Need more than 16 channels? Put this BEFORE your #include <GDDLink.h>:
//   #define GDDLINK_MAX_CHANNELS 24
#ifndef GDDLINK_MAX_CHANNELS
#define GDDLINK_MAX_CHANNELS 16
#endif

class GDDLink {
public:
  // ---- Switch I/O ("NAME:value") callbacks --------------------------
  using Callback = void (*)(const String &value);
  using Reader = String (*)();

  // ---- one callback per other screen ---------------------------------
  using HandCallback = void (*)(int ix, int iy, int tx, int ty, int pinch);
  using GestureCallback = void (*)(const String &name);
  using MotionCallback = void (*)(const String &name, int value);
  using ButtonCallback = void (*)(const String &btn, bool pressed);
  using StickCallback = void (*)(const String &side, float x, float y);
  using ServoCallback = void (*)(int channel, int angle);
  using VoiceCallback = void (*)(const String &text);
  using CarSpeedCallback = void (*)(int speed);
  using CarCommandCallback = void (*)(const String &cmd);

  // `port` is kept by reference — pass an already-declared Stream
  // (BluetoothSerial, Serial, Serial1, a SoftwareSerial, ...). Used for
  // outgoing send()/watch() traffic; incoming bytes are still read by your
  // own sketch and handed to feed(), see below.
  //
  // `terminator` ends an unframed message (Switch I/O "NAME:value", or a
  // Car/Tilt command like "F") — default '\n'. Framed messages like
  // "<G,FIST>" don't need it; they're delimited by '<' and '>' instead.
  // Only change this if your Car/Tilt sketch needs to match the app, which
  // sends those as ';'-terminated — see examples/Car and examples/Tilt.
  explicit GDDLink(Stream &port, char terminator = '\n');

  // Hand it one byte you just read yourself, e.g.:
  //   while (BT.available()) gdd.feed(BT.read());
  // Recognizes a "<...>" frame (Hand/Gamepad/Servo/Voice) or an unframed
  // token ending in `terminator` (Switch I/O "NAME:value", or a Car/Tilt
  // command), and fires whichever callback matches.
  void feed(char c);

  // Call once per loop(), unconditionally (it's cheap if nothing changed).
  // Checks every watch()-registered sender and sends "name:value" out the
  // instant a reading actually changes. Does NOT read incoming bytes —
  // that's feed()'s job.
  void poll();

  // Send "name:value\n" to the other side (Switch I/O / AI Chat).
  void send(const String &name, const String &value);
  void send(const String &name, long value);
  void send(const String &name, int value);
  void send(const String &name, float value, uint8_t decimals = 2);

  // ---- Hand screen -----------------------------------------------------
  // onGesture: a built-in pose (FIST, OPEN, POINT, PEACE, THREE, FOUR,
  //            THUMBSUP) or a name you trained in the app.
  // onMotion:  a trained continuous axis, e.g. name="Bend", value 0-100.
  // onHand:    raw wrist-relative coordinates, all five fields at once.
  void onGesture(GestureCallback cb);
  void onMotion(MotionCallback cb);
  void onHand(HandCallback cb);

  // ---- Gamepad screen ----------------------------------------------------
  // onButton: btn is the button name (e.g. "A"), pressed true on press.
  // onStick:  side is "L" or "R", x/y are -1.00..1.00.
  void onButton(ButtonCallback cb);
  void onStick(StickCallback cb);

  // ---- Servo Studio screen ---------------------------------------------
  // channel 1-4, angle already clamped to 0-180.
  void onServo(ServoCallback cb);

  // ---- Voice screen ------------------------------------------------------
  // text is whatever was said, as recognized speech-to-text.
  void onVoice(VoiceCallback cb);

  // ---- Car / Tilt screens -------------------------------------------------
  // Car sends a speed (0-255) then F/B/L/R/S commands; Tilt sends just
  // F/B/L/R/S (plus diagonals FR/FL/BR/BL, or custom text from its Edit
  // dialog) with no speed of its own. Use the terminator ';' constructor
  // for both — see examples/Car and examples/Tilt.
  void onCarSpeed(CarSpeedCallback cb);
  void onCarCommand(CarCommandCallback cb);

  // ---- Switch I/O / AI Chat ("NAME:value") ------------------------------
  // Last value received for `name` (empty string if never seen).
  String value(const String &name) const;
  long valueInt(const String &name) const;

  // Fire `cb(value)` whenever `name` is received. One callback per name.
  void onReceive(const String &name, Callback cb);

  // One-line output binding — call once in setup(). Sets pinMode
  // automatically; from then on every feed()'d "name:value" line
  // drives the pin directly, no callback needed.
  //   bindDigitalOut: "0"/anything-else -> LOW/HIGH
  //   bindAnalogOut:  "0".."255"        -> analogWrite (PWM), clamped
  void bindDigitalOut(const String &name, uint8_t pin);
  void bindAnalogOut(const String &name, uint8_t pin);

  // Auto-report a value to the app, but ONLY when it actually changes —
  // not on a timer, to keep the link cheap. `fn` is checked at most once
  // every `minIntervalMs` (rate-limits the read itself, e.g. for sensors
  // that are slow or shouldn't be polled every loop() pass); a send only
  // goes out of those checks when the returned text differs from the last
  // one actually sent. Call once in setup(); evaluated from poll().
  void watch(const String &name, Reader fn, unsigned long minIntervalMs = 200);

private:
  enum class Bind : uint8_t { none, digitalOut, analogOut };

  struct Channel {
    String name;
    String lastValue;
    Callback callback = nullptr;
    Bind bind = Bind::none;
    uint8_t pin = 0;
    Reader reader = nullptr;
    unsigned long minIntervalMs = 0;
    unsigned long lastCheckMs = 0;
    String lastSentValue;
    bool everSent = false;
  };

  Stream &_port;
  char _terminator;
  bool _inFrame = false;
  String _frameBuf;
  String _rxBuffer;
  Channel _channels[GDDLINK_MAX_CHANNELS]; // fixed at compile time, no heap
  uint8_t _count = 0;

  HandCallback _handCb = nullptr;
  GestureCallback _gestureCb = nullptr;
  MotionCallback _motionCb = nullptr;
  ButtonCallback _buttonCb = nullptr;
  StickCallback _stickCb = nullptr;
  ServoCallback _servoCb = nullptr;
  VoiceCallback _voiceCb = nullptr;
  CarSpeedCallback _carSpeedCb = nullptr;
  CarCommandCallback _carCommandCb = nullptr;

  Channel *_find(const String &name) const;
  Channel *_findOrCreate(const String &name);
  void _handleLine(String line);
  void _handleFrame(const String &frame);
  void _handleToken(const String &tok);
  void _evalWatches();
};
