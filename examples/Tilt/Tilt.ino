// ตัวอย่าง ESP32 — โหมด Tilt Control (ขับรถแบบเอียงมือถือ) ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ตัวจบข้อความเป็น ';' ตรงกับที่แอปส่งคำสั่งมา ตัวอักษรเดียว F B L R S เป็น
// ค่าเริ่มต้น bindCarMotors จัดการให้อัตโนมัติ ส่วนแนวทแยง (FR FL BR BL)
// หรือข้อความที่ตั้งเองได้ในหน้า Edit ไม่ตรงกับ F/B/L/R/S เลยไม่ขยับมอเตอร์
// เอง — ถ้าอยากให้รองรับด้วย เพิ่มเงื่อนไขใน onExtra() ตรงนี้

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT, ';');

const int ENA = 5, IN1 = 6, IN2 = 7, IN3 = 8, IN4 = 9, ENB = 10;

void onExtra(const String &cmd) {
  // if (cmd == "FR") { ... } // แนวทแยงหรือคำสั่งกำหนดเอง เพิ่มตรงนี้
}

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindCarMotors(ENA, IN1, IN2, IN3, IN4, ENB); // F/B/L/R/S จากการเอียงมือถือ
  gdd.onCarCommand(onExtra);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
