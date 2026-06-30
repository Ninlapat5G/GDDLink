// ---------------------------------------------------------------------------
// ตัวอย่าง GDDLink — Switch I/O ผ่าน Bluetooth Classic (ESP32)
//
// ตรงกับช่อง Switch I/O ตั้งต้นของแอป Arduino Bluetooth (GDD) พอดี
// ไม่ต้องตั้งอะไรเพิ่มฝั่งแอปเลย:
//
//   ช่อง "LED"   (Digital, 0/1)   ในแอป -> ขา 2 ของบอร์ดนี้
//   ช่อง "Motor" (Analog, 0-255)  ในแอป -> ขา 4 ของบอร์ดนี้ (PWM)
//   บอร์ดนี้รายงานค่า "Temp" กลับไปแอป — แต่ส่งเฉพาะตอนค่าเปลี่ยนจริง
//   ไม่ใช่ส่งทุกช่วงเวลาคงที่ แอปจะสร้างเป็นช่อง Sensor ให้เองอัตโนมัติ
//
// Flash ด้วย Arduino IDE (เลือกบอร์ด "ESP32 Dev Module") แล้วจับคู่มือถือ
// กับชื่อ "GDD-ESP32" จากนั้นเปิดหน้า Switch I/O หรือ AI Chat ในแอป
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);

const uint8_t LED_PIN = 2;
const uint8_t MOTOR_PIN = 4;

// เปลี่ยนเป็นอ่านเซนเซอร์จริงได้เลย เช่น `return String(analogRead(A0));`
String readTemp() {
  return String(25.0 + (random(-20, 20) / 10.0), 1);
}

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindDigitalOut("LED", LED_PIN);     // แอป -> ขา อัตโนมัติ
  gdd.bindAnalogOut("Motor", MOTOR_PIN);  // แอป -> ขา (PWM) อัตโนมัติ
  gdd.watch("Temp", readTemp);            // ขา -> แอป เฉพาะตอนค่าเปลี่ยน
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read()); // อ่านเองทีละไบต์ แล้วให้ gdd แปลงเป็น NAME:value
  }
  gdd.poll(); // เช็ค watch() — ส่ง "Temp:.." ทันทีที่ค่าเปลี่ยน
}
