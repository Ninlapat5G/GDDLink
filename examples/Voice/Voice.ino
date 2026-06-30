// ตัวอย่าง ESP32 — โหมด Voice (สั่งงานด้วยเสียง)
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// แอปแปลงเสียงเป็นข้อความแล้วส่งมาทั้งดุ้นเป็น <V,ข้อความที่พูด> —
// แค่หาคำสำคัญที่ต้องการในข้อความนั้นเอง

#include <BluetoothSerial.h>
BluetoothSerial BT;
const int RELAY_PIN = 4, LED_PIN = 2;

void setup() {
  BT.begin("GDD-ESP32");
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (BT.available()) {
    String c = BT.readStringUntil('>');   // เช่น "<V,เปิดไฟ"
    c.toLowerCase();
    if (c.indexOf("เปิด") >= 0 || c.indexOf("on") >= 0)  digitalWrite(RELAY_PIN, HIGH);
    if (c.indexOf("ปิด") >= 0 || c.indexOf("off") >= 0)  digitalWrite(RELAY_PIN, LOW);
    if (c.indexOf("กระพริบ") >= 0 || c.indexOf("blink") >= 0) {
      digitalWrite(LED_PIN, HIGH); delay(200);
      digitalWrite(LED_PIN, LOW);
    }
    // เพิ่มคำสั่งเสียงของตัวเองตรงนี้ได้เลย
  }
}
