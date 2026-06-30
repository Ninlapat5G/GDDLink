#include "GDDLink.h"

GDDLink::GDDLink(Stream &port, char terminator) : _port(port), _terminator(terminator) {}

GDDLink::Channel *GDDLink::_find(const String &name) const {
  for (uint8_t i = 0; i < _count; i++) {
    if (_channels[i].name == name) return const_cast<Channel *>(&_channels[i]);
  }
  return nullptr;
}

GDDLink::Channel *GDDLink::_findOrCreate(const String &name) {
  Channel *c = _find(name);
  if (c) return c;
  if (_count >= GDDLINK_MAX_CHANNELS) return nullptr; // full — ignore silently
  c = &_channels[_count++];
  c->name = name;
  return c;
}

void GDDLink::feed(char c) {
  if (c == _terminator) {
    _handleLine(_rxBuffer);
    _rxBuffer = "";
  } else if (c != '\r') {
    _rxBuffer += c;
  }
}

void GDDLink::poll() {
  _evalWatches();
}

void GDDLink::_evalWatches() {
  const unsigned long now = millis();
  for (uint8_t i = 0; i < _count; i++) {
    Channel &c = _channels[i];
    if (!c.reader) continue;
    if (now - c.lastCheckMs < c.minIntervalMs) continue;
    c.lastCheckMs = now;
    const String v = c.reader();
    if (!c.everSent || v != c.lastSentValue) {
      c.everSent = true;
      c.lastSentValue = v;
      send(c.name, v);
    }
  }
}

void GDDLink::_handleLine(String line) {
  line.trim();
  if (line.isEmpty()) return;
  const int sep = line.indexOf(':');
  if (sep <= 0) return; // not a "NAME:value" line — ignore

  String name = line.substring(0, sep);
  String val = line.substring(sep + 1);
  name.trim();
  val.trim();
  if (name.isEmpty()) return;

  Channel *ch = _findOrCreate(name);
  if (!ch) return;
  ch->lastValue = val;

  switch (ch->bind) {
    case Bind::digitalOut:
      digitalWrite(ch->pin, val.toInt() ? HIGH : LOW);
      break;
    case Bind::analogOut:
      analogWrite(ch->pin, constrain(val.toInt(), 0, 255));
      break;
    case Bind::none:
      break;
  }

  if (ch->callback) ch->callback(val);
}

void GDDLink::send(const String &name, const String &value) {
  _port.print(name);
  _port.print(':');
  _port.print(value);
  _port.print(_terminator);
}

void GDDLink::send(const String &name, long value) { send(name, String(value)); }
void GDDLink::send(const String &name, int value) { send(name, String(value)); }
void GDDLink::send(const String &name, float value, uint8_t decimals) {
  // Explicit (double, unsigned int) — uint8_t against the float/double/
  // integer overload set of String() is ambiguous on some cores (e.g. the
  // ESP32 Arduino core), since uint8_t can promote to several of them.
  send(name, String(static_cast<double>(value), static_cast<unsigned int>(decimals)));
}

String GDDLink::value(const String &name) const {
  Channel *c = _find(name);
  return c ? c->lastValue : String();
}

long GDDLink::valueInt(const String &name) const {
  Channel *c = _find(name);
  return c ? c->lastValue.toInt() : 0;
}

void GDDLink::onReceive(const String &name, Callback cb) {
  Channel *c = _findOrCreate(name);
  if (c) c->callback = cb;
}

void GDDLink::bindDigitalOut(const String &name, uint8_t pin) {
  Channel *c = _findOrCreate(name);
  if (!c) return;
  c->bind = Bind::digitalOut;
  c->pin = pin;
  pinMode(pin, OUTPUT);
}

void GDDLink::bindAnalogOut(const String &name, uint8_t pin) {
  Channel *c = _findOrCreate(name);
  if (!c) return;
  c->bind = Bind::analogOut;
  c->pin = pin;
  pinMode(pin, OUTPUT);
}

void GDDLink::watch(const String &name, Reader fn, unsigned long minIntervalMs) {
  Channel *c = _findOrCreate(name);
  if (!c) return;
  c->reader = fn;
  c->minIntervalMs = minIntervalMs;
  c->lastCheckMs = 0;
  c->everSent = false;
}
