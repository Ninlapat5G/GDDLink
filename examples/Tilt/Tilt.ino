// ตัวอย่าง ESP32 — โหมด Tilt Control (ขับรถแบบเอียงมือถือ)
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// แต่ละคำสั่งจบด้วย ';' เสมอ — ตัวอักษรเดียว F B L R S เป็นค่าเริ่มต้น แต่
// แอปยังส่งแนวทแยง (FR FL BR BL) หรือข้อความที่ตั้งเองได้ในหน้า Edit ด้วย

#include <BluetoothSerial.h>
BluetoothSerial BT;

const int ENA = 5, IN1 = 6, IN2 = 7, IN3 = 8, IN4 = 9, ENB = 10;
int spd = 170;
String buf;

void drive(const String &cmd);
void motor(int a, int b, int c, int d);

void setup() {
  BT.begin("GDD-ESP32");
  int pins[] = {ENA, IN1, IN2, IN3, IN4, ENB};
  for (int p : pins) pinMode(p, OUTPUT);
}

void loop() {
  while (BT.available()) {
    char c = BT.read();
    if (c == ';') { drive(buf); buf = ""; }
    else buf += c;
  }
}

void drive(const String &cmd) {
  analogWrite(ENA, spd); analogWrite(ENB, spd);
  if      (cmd == "F") motor(1, 0, 1, 0);   // เอียงขึ้น
  else if (cmd == "B") motor(0, 1, 0, 1);   // เอียงลง
  else if (cmd == "L") motor(0, 1, 1, 0);   // เอียงซ้าย
  else if (cmd == "R") motor(1, 0, 0, 1);   // เอียงขวา
  else if (cmd == "S") motor(0, 0, 0, 0);   // ตั้งตรง / หยุด
  // แนวทแยง (FR/FL/BR/BL) หรือคำสั่งกำหนดเอง — เพิ่ม "else if" ของตัวเองตรงนี้
}

void motor(int a, int b, int c, int d) {
  digitalWrite(IN1, a); digitalWrite(IN2, b);
  digitalWrite(IN3, c); digitalWrite(IN4, d);
}
