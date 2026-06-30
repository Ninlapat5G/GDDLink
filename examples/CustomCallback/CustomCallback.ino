// ---------------------------------------------------------------------------
// GDDLink example — custom logic with onReceive()
//
// bindDigitalOut()/bindAnalogOut() cover "one name -> one pin". For
// anything fancier (a name that should drive two pins, run a motor
// driver, etc.) register a callback instead and write your own logic.
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);

const uint8_t IN1 = 4, IN2 = 5; // simple H-bridge direction pins

void onFan(const String &v) {
  // App channel "Fan": "1" = forward, "0" = stop, "-1" = reverse
  const int dir = v.toInt();
  digitalWrite(IN1, dir > 0);
  digitalWrite(IN2, dir < 0);
}

void setup() {
  BT.begin("GDD-ESP32");
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  gdd.onReceive("Fan", onFan);
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read());
  }
  gdd.poll(); // no watch() here, but still cheap/safe to call every loop

  // You can also just read the last value yourself instead of a callback:
  // String fan = gdd.value("Fan");
}
