// ---------------------------------------------------------------------------
// ตัวอย่าง GDDLink — เขียน logic เองด้วย onReceive()
//
// bindDigitalOut()/bindAnalogOut() ใช้ได้กับเคส "ชื่อเดียว -> ขาเดียว"
// ถ้าต้องการอะไรซับซ้อนกว่านั้น (ชื่อเดียวคุมสองขา, สั่งงานมอเตอร์ไดรเวอร์
// ฯลฯ) ให้ลงทะเบียน callback แทน แล้วเขียน logic เองในนั้น
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);

const uint8_t IN1 = 4, IN2 = 5; // ขาคุมทิศทาง H-bridge อย่างง่าย

void onFan(const String &v) {
  // ช่อง "Fan" ในแอป: "1" = หมุนไปข้างหน้า, "0" = หยุด, "-1" = หมุนกลับ
  const int dir = v.toInt();
  digitalWrite(IN1, dir > 0);
  digitalWrite(IN2, dir < 0);
}

void setup() {
  BT.begin("GDD-ESP32");
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  gdd.onReceive("Fan", onFan);
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read());
  }
  gdd.poll(); // ตัวอย่างนี้ไม่มี watch() แต่เรียกทุกรอบไว้ก็ปลอดภัย ไม่เสียอะไร

  // จะอ่านค่าล่าสุดเองแทนการใช้ callback ก็ได้:
  // String fan = gdd.value("Fan");
}
