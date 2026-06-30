// ตัวอย่าง ESP32 — โหมด Servo Studio (สูงสุด 4 ตัว)
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ก่อนคอมไพล์ ติดตั้งไลบรารี "ESP32Servo" (โดย Kevin Harrington) ผ่าน
// Sketch -> Include Library -> Manage Libraries... ก่อน (ชิป ESP32 ไม่มี
// Servo.h ของ Arduino มาตรฐานมาให้ในตัว ต้องใช้ตัวนี้แทน)
//
// รับ: <S,channel,angle>   channel 1-4, angle 0-180

#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial BT;
Servo servos[4];
const int PINS[4] = {18, 19, 21, 22}; // ขาที่ต่อ servo แต่ละช่อง (ch1..ch4)

int nextInt(String &s);

void setup() {
  BT.begin("GDD-ESP32");
  for (int i = 0; i < 4; i++) {
    servos[i].attach(PINS[i]);
    servos[i].write(90);
  }
}

void loop() {
  if (BT.available()) {
    String f = BT.readStringUntil('>');
    if (f.indexOf("<S,") < 0) return;
    f = f.substring(f.indexOf("<S,") + 3);
    int ch  = nextInt(f);                  // 1-4
    int ang = constrain(nextInt(f), 0, 180);
    if (ch >= 1 && ch <= 4) servos[ch - 1].write(ang);
  }
}

int nextInt(String &s) {
  int c = s.indexOf(',');
  int v = (c < 0 ? s : s.substring(0, c)).toInt();
  s = (c < 0) ? "" : s.substring(c + 1);
  return v;
}
