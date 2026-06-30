// ตัวอย่าง ESP32 — โหมด Hand (มือ/ท่าทาง)
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ก่อนคอมไพล์ ติดตั้งไลบรารี "ESP32Servo" (โดย Kevin Harrington) ผ่าน
// Sketch -> Include Library -> Manage Libraries... ก่อน (ชิป ESP32 ไม่มี
// Servo.h ของ Arduino มาตรฐานมาให้ในตัว ต้องใช้ตัวนี้แทน)
//
// แอปส่งข้อมูลมาได้ 3 แบบ:
//   <G,ix,iy,tx,ty,pinch>   ตำแหน่งดิบ 5 ค่า (โหมด RAW)
//   <G,NAME>                ท่าทางสำเร็จรูปหรือที่ฝึกเอง เช่น <G,FIST>
//   <G,NAME,0-100>          แกนที่ฝึกแบบต่อเนื่อง (โหมด Motion) เช่น <G,Bend,72>

#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial BT;
Servo servoX, servoY;
const int LED_PIN = 2;

void handleNamed(const String &name);
void handleAxis(const String &name, int value);
String nextField(String &s);

void setup() {
  BT.begin("GDD-ESP32");
  servoX.attach(18);
  servoY.attach(19);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (BT.available()) {
    String p = BT.readStringUntil('>');
    int g = p.indexOf("<G,");
    if (g < 0) return;
    p = p.substring(g + 3); // ตัด "<G," ทิ้ง

    // นับจำนวน comma เพื่อแยก 3 รูปแบบออกจากกัน
    unsigned int commas = 0;
    for (unsigned int i = 0; i < p.length(); i++) if (p[i] == ',') commas++;

    if (commas == 0) {
      handleNamed(p);                      // <G,NAME>
    } else if (commas == 1) {
      String name = nextField(p);
      handleAxis(name, p.toInt());         // <G,NAME,0-100>
    } else {
      int ix = nextField(p).toInt();       // <G,ix,iy,tx,ty,pinch>
      int iy = nextField(p).toInt();
      servoX.write(map(ix, -100, 100, 0, 180));
      servoY.write(map(iy, -100, 100, 0, 180));
    }
  }
}

// ท่าทางสำเร็จรูป (FIST, OPEN, POINT, PEACE, THREE, FOUR, THUMBSUP) หรือชื่อ
// ที่ตั้งเองตอนฝึกในแอป — เพิ่มเงื่อนไขของตัวเองต่อท้ายแถวนี้ได้เลย
void handleNamed(const String &name) {
  if (name == "FIST") { digitalWrite(LED_PIN, HIGH); return; }
  if (name == "OPEN") { digitalWrite(LED_PIN, LOW);  return; }
  // เพิ่มท่าทางอื่นที่ต้องการตรงนี้
}

// แกนต่อเนื่องจากโหมด Motion เช่น <G,Bend,0-100>
void handleAxis(const String &name, int value) {
  if (name == "Bend") servoX.write(map(value, 0, 100, 0, 180));
}

// ดึง field แรกที่คั่นด้วย comma ออกจาก s แล้วคืนค่ากลับมา
String nextField(String &s) {
  int c = s.indexOf(',');
  String v = (c < 0) ? s : s.substring(0, c);
  s = (c < 0) ? "" : s.substring(c + 1);
  return v;
}
