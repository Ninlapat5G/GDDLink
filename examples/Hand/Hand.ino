// ตัวอย่าง ESP32 — โหมด Hand (มือ/ท่าทาง) ใช้ GDDLink
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// ก่อนคอมไพล์ ติดตั้งไลบรารี "ESP32Servo" (โดย Kevin Harrington) ผ่าน
// Sketch -> Include Library -> Manage Libraries... ก่อน (ชิป ESP32 ไม่มี
// Servo.h ของ Arduino มาตรฐานมาให้ในตัว ต้องใช้ตัวนี้แทน)

#include <BluetoothSerial.h>
#include <ESP32Servo.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);
Servo servoX, servoY;
const int LED_PIN = 2;

// แกนต่อเนื่องจากโหมด Motion เช่น name="Bend" value 0-100 — ต้อง map()
// ค่าเอง เลยยังเป็น callback อยู่
void onMotion(const String &name, int value) {
  if (name == "Bend") servoX.write(map(value, 0, 100, 0, 180));
}

// โหมด RAW — พิกัดมือดิบ 5 ค่า ต้อง map() เข้า servo 2 แกน เลยยังเป็น
// callback อยู่เหมือนกัน
void onHand(int ix, int iy, int tx, int ty, int pinch) {
  servoX.write(map(ix, -100, 100, 0, 180));
  servoY.write(map(iy, -100, 100, 0, 180));
}

void setup() {
  BT.begin("GDD-ESP32");
  servoX.attach(18);
  servoY.attach(19);

  // ท่าทางสำเร็จรูป/ที่ฝึกเอง -> ขาตรงๆ ไม่ต้องเขียนฟังก์ชันเอง เพิ่มท่าอื่น
  // ได้โดยเรียกซ้ำอีกบรรทัดด้วยชื่อท่าใหม่
  gdd.bindGestureDigitalOut("FIST", LED_PIN, HIGH);
  gdd.bindGestureDigitalOut("OPEN", LED_PIN, LOW);

  gdd.onMotion(onMotion);
  gdd.onHand(onHand);
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
