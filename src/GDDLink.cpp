#include "GDDLink.h"

namespace {

bool isAllDigits(const String &s) {
  if (s.isEmpty()) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    if (!isDigit(s[i])) return false;
  }
  return true;
}

// Pulls the next comma-separated field off the front of s, consuming it.
String nextField(String &s) {
  int c = s.indexOf(',');
  String v = (c < 0) ? s : s.substring(0, c);
  s = (c < 0) ? "" : s.substring(c + 1);
  return v;
}

// nth comma-separated field of f (0-based), without consuming it.
String field(const String &f, int n) {
  int start = 0;
  for (int i = 0; i < n; i++) {
    int c = f.indexOf(',', start);
    if (c < 0) return "";
    start = c + 1;
  }
  int c = f.indexOf(',', start);
  return (c < 0) ? f.substring(start) : f.substring(start, c);
}

} // namespace

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
  if (c == '<') {
    _inFrame = true;
    _frameBuf = "";
    return;
  }
  if (_inFrame) {
    if (c == '>') {
      _inFrame = false;
      _handleFrame(_frameBuf);
    } else {
      _frameBuf += c;
    }
    return;
  }
  if (c == _terminator) {
    if (_rxBuffer.length()) {
      _handleToken(_rxBuffer);
      _rxBuffer = "";
    }
    return;
  }
  if (c != '\r' && c != '\n') {
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

// A complete "<...>" frame body (without the brackets) — Hand, Gamepad,
// Servo or Voice. Dispatched by its leading letter, same as the app's own
// wire format: G=Hand, B=button, J=stick, S=servo, V=voice.
void GDDLink::_handleFrame(const String &f) {
  if (f.isEmpty()) return;
  switch (f[0]) {
    case 'G': {
      String body = f.substring(2); // after "G,"
      unsigned int commas = 0;
      for (unsigned int i = 0; i < body.length(); i++) {
        if (body[i] == ',') commas++;
      }
      if (commas == 0) {
        if (_gestureCb) _gestureCb(body);
      } else if (commas == 1) {
        String name = nextField(body);
        if (_motionCb) _motionCb(name, body.toInt());
      } else if (_handCb) {
        int ix = nextField(body).toInt();
        int iy = nextField(body).toInt();
        int tx = nextField(body).toInt();
        int ty = nextField(body).toInt();
        int pinch = body.toInt();
        _handCb(ix, iy, tx, ty, pinch);
      }
      break;
    }
    case 'B':
      if (_buttonCb) _buttonCb(field(f, 1), field(f, 2) == "1");
      break;
    case 'J':
      if (_stickCb) _stickCb(field(f, 1), field(f, 2).toFloat(), field(f, 3).toFloat());
      break;
    case 'S':
      if (_servoCb) _servoCb(field(f, 1).toInt(), constrain(field(f, 2).toInt(), 0, 180));
      break;
    case 'V': {
      int c = f.indexOf(',');
      if (_voiceCb) _voiceCb(c < 0 ? String() : f.substring(c + 1));
      break;
    }
    default:
      break; // unknown frame type — ignore
  }
}

// A complete unframed token (no leading '<') — Switch I/O "NAME:value", or
// a Car/Tilt command. A ':' means Switch I/O; otherwise it's a car token.
void GDDLink::_handleToken(const String &tok) {
  if (tok.isEmpty()) return;
  if (tok.indexOf(':') > 0) {
    _handleLine(tok);
    return;
  }
  if (tok[0] == 'V' && tok.length() > 1 && isAllDigits(tok.substring(1))) {
    if (_carSpeedCb) _carSpeedCb(tok.substring(1).toInt());
    return;
  }
  if (_carCommandCb) _carCommandCb(tok);
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

void GDDLink::onGesture(GestureCallback cb) { _gestureCb = cb; }
void GDDLink::onMotion(MotionCallback cb) { _motionCb = cb; }
void GDDLink::onHand(HandCallback cb) { _handCb = cb; }
void GDDLink::onButton(ButtonCallback cb) { _buttonCb = cb; }
void GDDLink::onStick(StickCallback cb) { _stickCb = cb; }
void GDDLink::onServo(ServoCallback cb) { _servoCb = cb; }
void GDDLink::onVoice(VoiceCallback cb) { _voiceCb = cb; }
void GDDLink::onCarSpeed(CarSpeedCallback cb) { _carSpeedCb = cb; }
void GDDLink::onCarCommand(CarCommandCallback cb) { _carCommandCb = cb; }

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
