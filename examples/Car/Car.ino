// ตัวอย่าง ESP32 — โหมด Car Controller (มอเตอร์ขับ L298N) ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ตัวจบข้อความเป็น ';' ตรงกับที่แอปส่งคำสั่งรถมา (ตั้งเป็น default ของ
// GDDLink ไม่ได้ เพราะ Switch I/O ใช้ '\n' — ใส่เองตรง constructor)

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT, ';');

const int ENA = 5, IN1 = 6, IN2 = 7, IN3 = 8, IN4 = 9, ENB = 10;

// เพิ่มคำสั่งอื่นได้ เช่น "H" (แตร) หรือ "X1"/"X0" (ไฟ) — bindCarMotors
// จัดการแค่ F/B/L/R/S กับความเร็วให้อัตโนมัติ ที่เหลือมาที่นี่
void onExtra(const String &cmd) {
  // if (cmd == "H") { ... }
}

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindCarMotors(ENA, IN1, IN2, IN3, IN4, ENB); // F/B/L/R/S + ความเร็ว อัตโนมัติ
  gdd.onCarCommand(onExtra);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
