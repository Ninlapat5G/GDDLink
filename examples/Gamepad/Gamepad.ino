// ตัวอย่าง ESP32 — โหมด Gamepad ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);
const int LED_PIN = 2;

// btn คือชื่อปุ่ม เช่น "A", pressed = true ตอนกด
void onButton(const String &btn, bool pressed) {
  if (btn == "A") digitalWrite(LED_PIN, pressed ? HIGH : LOW);
  Serial.println(btn + String(pressed ? " กด" : " ปล่อย"));
  // เพิ่มปุ่มอื่นที่ต้องการตรงนี้ เช่น if (btn == "B") ...
}

// side คือ "L" หรือ "R", x และ y อยู่ในช่วง -1.00 ถึง 1.00
void onStick(const String &side, float x, float y) {
  Serial.println(side + " x=" + String(x) + " y=" + String(y));
  // เพิ่ม logic ขับมอเตอร์ตามค่า x, y ของตัวเองตรงนี้ได้
}

void setup() {
  BT.begin("GDD-ESP32");
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  gdd.onButton(onButton);
  gdd.onStick(onStick);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
