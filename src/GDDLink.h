// ---------------------------------------------------------------------------
// GDDLink — the easiest way to talk to the Arduino Bluetooth (GDD) app.
//
// The app's Switch I/O screen and AI chat both speak one tiny protocol:
// a line of text shaped "NAME:value\n" (e.g. "LED:1", "Temp:25.4"). You
// still read incoming bytes yourself with the normal Arduino pattern
// (if/while available() + read()) — this library only turns the bytes you
// hand it into "NAME:value" and drives the matching pin/callback/value:
//
//   #include <GDDLink.h>
//   GDDLink gdd(BT);                  // BT = any Stream: BluetoothSerial,
//                                      // HardwareSerial, SoftwareSerial...
//                                      // (NOT named "link" — that collides
//                                      // with a libc symbol on ESP32 boards)
//   String readTemp() { return String(analogRead(A0)); }
//   void setup() {
//     BT.begin("GDD-ESP32");
//     gdd.bindDigitalOut("LED", 2);   // app's "LED" channel drives pin 2
//     gdd.watch("Temp", readTemp);    // auto-reports "Temp" — but only
//   }                                 // when the reading actually changes
//   void loop() {
//     while (BT.available()) {
//       gdd.feed(BT.read());          // you read the bytes, gdd parses them
//     }
//     gdd.poll();                     // call every loop — sends watch()ed
//   }                                 // values out the moment they change
//
// See examples/SwitchIO for the full three-channel demo (LED, Motor, Temp)
// that matches the app's default channels out of the box.
// ---------------------------------------------------------------------------

#pragma once

#include <Arduino.h>

// How many distinct channel names one GDDLink can track, fixed at compile
// time — no heap allocation, so it's safe on small AVR boards (Uno/Nano
// only have 2 KB of RAM; a `new`'d array risks heap fragmentation there).
// Need more than 16 channels? Put this BEFORE your #include <GDDLink.h>:
//   #define GDDLINK_MAX_CHANNELS 24
#ifndef GDDLINK_MAX_CHANNELS
#define GDDLINK_MAX_CHANNELS 16
#endif

class GDDLink {
public:
  // Callback fired with the raw value text whenever `name` is received.
  using Callback = void (*)(const String &value);

  // Reads a current value to (maybe) send out. Returning the same text as
  // last time is fine — watch() only actually sends when it changes.
  using Reader = String (*)();

  // `port` is kept by reference — pass an already-declared Stream
  // (BluetoothSerial, Serial, Serial1, a SoftwareSerial, ...). Used for
  // outgoing send()/watch() traffic; incoming bytes are still read by your
  // own sketch and handed to feed(), see below.
  //
  // `terminator` is the byte that ends a "NAME:value" message, both ways —
  // default '\n' matches the usual Arduino convention (println(), readStringUntil('\n')).
  // Change it if your project already uses something else to separate
  // messages on the same link (e.g. ';' if you're sharing the stream with
  // another unframed protocol that has no terminator of its own).
  explicit GDDLink(Stream &port, char terminator = '\n');

  // Hand it one byte you just read yourself, e.g.:
  //   while (BT.available()) gdd.feed(BT.read());
  // Builds up a line; once a complete "NAME:value" line arrives, parses
  // it, updates the stored value, drives any pin-bound channel and fires
  // any registered callback.
  void feed(char c);

  // Call once per loop(), unconditionally (it's cheap if nothing changed).
  // Checks every watch()-registered sender and sends "name:value" out the
  // instant a reading actually changes. Does NOT read incoming bytes —
  // that's feed()'s job.
  void poll();

  // Send "name:value\n" to the other side.
  void send(const String &name, const String &value);
  void send(const String &name, long value);
  void send(const String &name, int value);
  void send(const String &name, float value, uint8_t decimals = 2);

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
  String _rxBuffer;
  Channel _channels[GDDLINK_MAX_CHANNELS]; // fixed at compile time, no heap
  uint8_t _count = 0;

  Channel *_find(const String &name) const;
  Channel *_findOrCreate(const String &name);
  void _handleLine(String line);
  void _evalWatches();
};
