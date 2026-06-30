// ตัวอย่าง ESP32 — โหมด Car Controller (มอเตอร์ขับ L298N)
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// รับโทเค็นที่จบด้วย ';' เสมอ: V0..V255;  แล้วตามด้วย F; B; L; R; S;

#include <BluetoothSerial.h>
BluetoothSerial BT;

const int ENA = 5, IN1 = 6, IN2 = 7, IN3 = 8, IN4 = 9, ENB = 10;
int spd = 160;
String buf;

void handleToken(const String &tok);
void drive(char c);
void motor(int a, int b, int c, int d);

void setup() {
  BT.begin("GDD-ESP32");
  int pins[] = {ENA, IN1, IN2, IN3, IN4, ENB};
  for (int p : pins) pinMode(p, OUTPUT);
}

void loop() {
  while (BT.available()) {
    char c = BT.read();
    if (c == ';') { handleToken(buf); buf = ""; }
    else buf += c;
  }
}

void handleToken(const String &tok) {
  if (tok.length() && tok[0] == 'V') { spd = tok.substring(1).toInt(); return; }
  if (tok.length() == 1) drive(tok[0]);
  // เพิ่มคำสั่งอื่น เช่น "H" (แตร) หรือ "X1"/"X0" (ไฟ) ตรงนี้ได้
}

void drive(char c) {
  analogWrite(ENA, spd); analogWrite(ENB, spd);
  switch (c) {
    case 'F': motor(1, 0, 1, 0); break;   // เดินหน้า
    case 'B': motor(0, 1, 0, 1); break;   // ถอยหลัง
    case 'L': motor(0, 1, 1, 0); break;   // เลี้ยวซ้าย
    case 'R': motor(1, 0, 0, 1); break;   // เลี้ยวขวา
    default:  motor(0, 0, 0, 0);          // S = หยุด
  }
}

void motor(int a, int b, int c, int d) {
  digitalWrite(IN1, a); digitalWrite(IN2, b);
  digitalWrite(IN3, c); digitalWrite(IN4, d);
}
