// ---------------------------------------------------------------------------
// ตัวอย่าง GDDLink — ใช้ร่วมกับโหมดควบคุมอื่นๆ ของแอปในไฟล์เดียว (ครบทุกโหมด)
//
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools → Board → "ESP32 Dev Module")
// ไม่ใช่โปรเจกต์ PlatformIO — แค่ไฟล์ .ino ไฟล์เดียวจบ ไม่ต้องตั้งค่าอะไรเพิ่ม
//
// GDDLink ดูแลแค่ Switch I/O / AI Chat (ข้อความ "NAME:value") — โหมดอื่นของ
// แอป (Hand, Gamepad, Servo, Car, Tilt, Voice) ไม่เกี่ยวกับ GDDLink เลย
// สเก็ตช์นี้แสดงให้เห็นว่าจะเขียน parser ของตัวเองคู่กับ GDDLink ในไฟล์
// เดียวกันได้อย่างไร โดยไม่ชนกัน:
//
//   มีกรอบ <...>   : Hand <G,..> · Gamepad <B,..>/<J,..> · Servo <S,..>
//                    · Voice <V,..>  — แยกขอบเขตด้วย < > อยู่แล้ว ไม่กำกวม
//   ไม่มีกรอบ      : Switch I/O "NAME:value;" (ผ่าน GDDLink) และคำสั่ง
//                    Car/Tilt ตัวอักษรสั้นๆ (F; B; L; R; S; ฯลฯ)
//
// กฎสำคัญ: ทุกอย่างที่ "ไม่มีกรอบ" ต้องจบด้วย ';' เหมือนกันหมด — กันไม่ให้
// "LED:1;" ปนกับคำสั่งรถตัวอักษรเดียว "F;" (ดูรายละเอียดที่ README หัวข้อ
// "เปลี่ยนตัวจบข้อความ")
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT, ';'); // ';' แทน '\n' ดีฟอลต์ — ให้ตรงกับคำสั่ง Car/Tilt ด้านล่าง

const uint8_t LED_PIN = 2;    // ช่อง Switch I/O "LED"
const uint8_t MOTOR_PIN = 4;  // ช่อง Switch I/O "Motor" (PWM)
const uint8_t ENA = 5, IN1 = 6, IN2 = 7, IN3 = 8, IN4 = 9, ENB = 10; // Car/Tilt (L298N)

bool inFrame = false;
String frame;   // สะสมตัวอักษรระหว่าง '<' กับ '>'
String rawBuf;  // สะสมตัวอักษรนอกกรอบ ยังไม่ตัดสินใจว่าเป็นอะไร
int carSpeed = 160;

// ประกาศฟังก์ชันล่วงหน้า (Arduino IDE เติมให้อัตโนมัติปกติ แต่เขียนเองไว้
// ชัดเจนกว่า เผื่อเอาไปคอมไพล์ด้วยเครื่องมืออื่นที่ไม่มีฟีเจอร์นี้)
void handleFrame(const String &f);
void handleRawToken(const String &tok);
void drive(const String &cmd);
void motor(int a, int b, int c, int d);

void setup() {
  Serial.begin(115200);
  BT.begin("GDD-ESP32");

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  int carPins[] = {ENA, IN1, IN2, IN3, IN4, ENB};
  for (int p : carPins) pinMode(p, OUTPUT);

  // Switch I/O / AI Chat — ใช้ GDDLink ตามปกติ เหมือนตัวอย่าง SwitchIO
  gdd.bindDigitalOut("LED", LED_PIN);
  gdd.bindAnalogOut("Motor", MOTOR_PIN);
  gdd.watch("LED", []() { return String(digitalRead(LED_PIN)); });

  Serial.println("พร้อมแล้ว — จับคู่กับ \"GDD-ESP32\" แล้วลองทุกโหมดในแอปได้เลย");
}

void loop() {
  while (BT.available()) {
    char c = BT.read();
    if (c == '<') {
      inFrame = true;
      frame = "";
    } else if (inFrame) {
      if (c == '>') { inFrame = false; handleFrame(frame); }
      else frame += c;
    } else if (c == ';') {
      if (rawBuf.length()) { handleRawToken(rawBuf); rawBuf = ""; }
    } else if (c != '\r' && c != '\n') {
      rawBuf += c;
    }
  }
  gdd.poll(); // ส่งช่องที่ watch() ไว้กลับไปแอปทันทีที่ค่าเปลี่ยน
}

// ---- โหมดที่มีกรอบ <...> : Hand, Gamepad, Servo, Voice ----
// (ไม่เกี่ยวกับ GDDLink เลย — เขียน parser เองตามปกติ)

String field(const String &f, int n) {
  int start = 0;
  for (int i = 0; i < n; i++) {
    int c = f.indexOf(',', start);
    if (c < 0) return "";
    start = c + 1;
  }
  int c = f.indexOf(',', start);
  return (c < 0) ? f.substring(start) : f.substring(start, c);
}

void handleFrame(const String &f) {
  const char type = f.length() ? f[0] : '?';
  switch (type) {
    case 'G': // Hand: <G,ix,iy,tx,ty,pinch> หรือ <G,NAME>
      Serial.println("[HAND] " + f.substring(2));
      break;
    case 'B': // Gamepad ปุ่ม: <B,btn,state>
      Serial.println("[PAD] button " + field(f, 1) + " = " + field(f, 2));
      break;
    case 'J': // Gamepad สติ๊ก: <J,side,x,y>
      Serial.println("[PAD] stick " + field(f, 1) + " x=" + field(f, 2) + " y=" + field(f, 3));
      break;
    case 'S': { // Servo: <S,channel,angle> — ต่อ Servo จริงแล้วสั่งงานตรงนี้ได้เลย
      int ch = field(f, 1).toInt();
      int angle = constrain(field(f, 2).toInt(), 0, 180);
      Serial.println("[SERVO] channel " + String(ch) + " angle=" + String(angle));
      break;
    }
    case 'V': // Voice: <V,คำพูด>
      Serial.println("[VOICE] " + field(f, 1));
      break;
  }
}

// ---- โหมดที่ไม่มีกรอบ : Switch I/O (ผ่าน GDDLink) + Car/Tilt ----

// โทเค็นที่จบด้วย ';' หนึ่งตัว — เป็น "NAME:value" (มี ':') หรือคำสั่ง
// Car/Tilt สั้นๆ (ไม่มี ':') ตัดสินใจจากเนื้อหา ไม่ใช่จากว่าใครส่งมา
void handleRawToken(const String &tok) {
  if (tok.isEmpty()) return;

  if (tok.indexOf(':') > 0) {
    // Switch I/O / AI Chat — ส่งทีละตัวอักษรให้ GDDLink จัดการต่อ
    for (unsigned int i = 0; i < tok.length(); i++) gdd.feed(tok[i]);
    gdd.feed(';');
    return;
  }

  if (tok[0] == 'V' && tok.length() > 1) { carSpeed = tok.substring(1).toInt(); return; }
  if (tok == "H") { Serial.println("[CAR] HORN"); return; }
  if (tok == "X0" || tok == "X1") { Serial.println("[CAR] lights " + tok); return; }
  drive(tok); // F/B/L/R/S และ diagonal ของ Tilt (FR/FL/BR/BL) หรือคำสั่งกำหนดเอง
}

void drive(const String &cmd) {
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  if      (cmd == "F") motor(1, 0, 1, 0);
  else if (cmd == "B") motor(0, 1, 0, 1);
  else if (cmd == "L") motor(0, 1, 1, 0);
  else if (cmd == "R") motor(1, 0, 0, 1);
  else if (cmd == "S") motor(0, 0, 0, 0);
  else Serial.println("[CAR] custom command: " + cmd); // เช่น diagonal — เพิ่ม case เองได้
}

void motor(int a, int b, int c, int d) {
  digitalWrite(IN1, a); digitalWrite(IN2, b);
  digitalWrite(IN3, c); digitalWrite(IN4, d);
}
