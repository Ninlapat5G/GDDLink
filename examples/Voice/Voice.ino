// ตัวอย่าง ESP32 — โหมด Voice (สั่งงานด้วยเสียง) ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);
const int RELAY_PIN = 4, LED_PIN = 2;

// text คือสิ่งที่พูด แปลงเป็นข้อความมาแล้ว — แค่หาคำสำคัญที่ต้องการ
void onVoice(const String &text) {
  String c = text;
  c.toLowerCase();
  if (c.indexOf("เปิด") >= 0 || c.indexOf("on") >= 0)  digitalWrite(RELAY_PIN, HIGH);
  if (c.indexOf("ปิด") >= 0 || c.indexOf("off") >= 0)  digitalWrite(RELAY_PIN, LOW);
  if (c.indexOf("กระพริบ") >= 0 || c.indexOf("blink") >= 0) {
    digitalWrite(LED_PIN, HIGH); delay(200);
    digitalWrite(LED_PIN, LOW);
  }
  // เพิ่มคำสั่งเสียงของตัวเองตรงนี้ได้เลย
}

void setup() {
  BT.begin("GDD-ESP32");
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  gdd.onVoice(onVoice);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
