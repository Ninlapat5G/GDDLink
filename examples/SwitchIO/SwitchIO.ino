// ---------------------------------------------------------------------------
// GDDLink example — Switch I/O over Bluetooth Classic (ESP32)
//
// Matches the Arduino Bluetooth (GDD) app's default Switch I/O channels
// out of the box — no setup needed on the app side:
//
//   App channel "LED"   (Digital, 0/1)   -> this board's pin 2
//   App channel "Motor" (Analog, 0-255)  -> this board's pin 4 (PWM)
//   This board reports "Temp" to the app — but only when the reading
//   actually changes, not on a fixed timer. The app auto-discovers it as
//   a Sensor channel.
//
// Flash this with the Arduino IDE (Board: "ESP32 Dev Module"), then pair
// the phone with "GDD-ESP32" and open Switch I/O or AI Chat in the app.
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);

const uint8_t LED_PIN = 2;
const uint8_t MOTOR_PIN = 4;

// Swap this for a real sensor read, e.g. `return String(analogRead(A0));`
String readTemp() {
  return String(25.0 + (random(-20, 20) / 10.0), 1);
}

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindDigitalOut("LED", LED_PIN);     // app -> pin, automatic
  gdd.bindAnalogOut("Motor", MOTOR_PIN);  // app -> pin (PWM), automatic
  gdd.watch("Temp", readTemp);            // pin -> app, only when it changes
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read()); // you read the bytes; gdd turns them into NAME:value
  }
  gdd.poll(); // checks watch() — sends "Temp:.." the instant it changes
}
