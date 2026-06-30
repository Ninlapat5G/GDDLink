// ตัวอย่าง ESP32 — โหมด Servo Studio (สูงสุด 4 ตัว) ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ก่อนคอมไพล์ ติดตั้งไลบรารี "ESP32Servo" (โดย Kevin Harrington) ผ่าน
// Sketch -> Include Library -> Manage Libraries... ก่อน (ชิป ESP32 ไม่มี
// Servo.h ของ Arduino มาตรฐานมาให้ในตัว ต้องใช้ตัวนี้แทน)

#include <BluetoothSerial.h>
#include <ESP32Servo.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);
Servo servos[4];
const int PINS[4] = {18, 19, 21, 22}; // ขาที่ต่อ servo แต่ละช่อง (ch1..ch4)

// channel 1-4, angle ถูก clamp 0-180 มาให้แล้ว
void onServo(int channel, int angle) {
  if (channel >= 1 && channel <= 4) servos[channel - 1].write(angle);
}

void setup() {
  BT.begin("GDD-ESP32");
  for (int i = 0; i < 4; i++) {
    servos[i].attach(PINS[i]);
    servos[i].write(90);
  }
  gdd.onServo(onServo);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
