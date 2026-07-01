// ตัวอย่าง ESP32 — โหมด Gamepad ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);
const int LED_PIN = 2;

// สติ๊กอนาล็อกต้องมี logic เอง (ขับมอเตอร์แต่ละแบบไม่เหมือนกัน) เลยยังเป็น
// callback อยู่ — side คือ "L" หรือ "R", x และ y อยู่ในช่วง -1.00 ถึง 1.00
void onStick(const String &side, float x, float y) {
  Serial.println(side + " x=" + String(x) + " y=" + String(y));
  // เพิ่ม logic ขับมอเตอร์ตามค่า x, y ของตัวเองตรงนี้ได้
}

void setup() {
  BT.begin("GDD-ESP32");
  Serial.begin(115200);

  gdd.bindButtonDigitalOut("A", LED_PIN); // ปุ่ม A -> ขา LED ตรงๆ ไม่ต้องเขียนฟังก์ชันเอง
  gdd.onStick(onStick);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
