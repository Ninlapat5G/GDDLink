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
int spd = 160;

void motor(int a, int b, int c, int d) {
  digitalWrite(IN1, a); digitalWrite(IN2, b);
  digitalWrite(IN3, c); digitalWrite(IN4, d);
}

void onCarSpeed(int speed) { spd = speed; }

void onCarCommand(const String &cmd) {
  analogWrite(ENA, spd); analogWrite(ENB, spd);
  if      (cmd == "F") motor(1, 0, 1, 0);   // เดินหน้า
  else if (cmd == "B") motor(0, 1, 0, 1);   // ถอยหลัง
  else if (cmd == "L") motor(0, 1, 1, 0);   // เลี้ยวซ้าย
  else if (cmd == "R") motor(1, 0, 0, 1);   // เลี้ยวขวา
  else if (cmd == "S") motor(0, 0, 0, 0);   // หยุด
  // "H" (แตร), "X1"/"X0" (ไฟ) หรือคำสั่งอื่น — เพิ่ม "else if" ตรงนี้
}

void setup() {
  BT.begin("GDD-ESP32");
  int pins[] = {ENA, IN1, IN2, IN3, IN4, ENB};
  for (int p : pins) pinMode(p, OUTPUT);

  gdd.onCarSpeed(onCarSpeed);
  gdd.onCarCommand(onCarCommand);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
