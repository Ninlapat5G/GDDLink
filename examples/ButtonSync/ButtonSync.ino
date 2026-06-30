// ---------------------------------------------------------------------------
// ตัวอย่าง GDDLink — ปุ่มกดจริงเปลี่ยน LED แล้วซิงค์สถานะกลับไปแอปด้วย
//
// ผูกชื่อ "LED" สองทางพร้อมกันบนชื่อช่องเดียวกัน:
//   bindDigitalOut("LED", LED_PIN)  -> แอปสั่ง "LED:1"/"LED:0" มา ก็ขยับขาให้
//   watch("LED", ...)               -> ขาเปลี่ยนเมื่อไหร่ (ไม่ว่าใครเป็นคนสั่ง)
//                                      ก็ส่งสถานะจริงกลับไปแอปให้เอง
//
// กดปุ่มที่ต่อกับ BTN_PIN -> โค้ดสลับ LED เอง (ไม่ผ่านแอป) -> แอปจะเห็นสถานะ
// LED อัปเดตตามจริงทันที โดยไม่ต้องเขียนอะไรเพิ่มฝั่งส่งเอง
// ---------------------------------------------------------------------------

#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);

const uint8_t LED_PIN = 2;  // LED บนบอร์ด (ESP32 WROOM ส่วนใหญ่)
const uint8_t BTN_PIN = 5;  // ปุ่มกด ต่อขาหนึ่งลง GND อีกขาเข้า BTN_PIN

void setup() {
  BT.begin("GDD-ESP32");
  pinMode(BTN_PIN, INPUT_PULLUP);

  gdd.bindDigitalOut("LED", LED_PIN); // แอป -> ขา (สั่งงาน)

  // ขา -> แอป (รายงานสถานะจริง) — ฟังก์ชันแค่บรรทัดเดียว เลยเขียนสดตรงนี้เลย
  // ไม่ต้องแยกไปตั้งชื่อฟังก์ชันให้งง
  gdd.watch("LED", []() { return String(digitalRead(LED_PIN)); });
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read());
  }

  // ตรวจจับ "ขอบขา" ของการกด (กดครั้งเดียว สลับครั้งเดียว) ไม่ใช้ delay()
  // กันค้าง เพราะ delay() จะทำให้พลาดข้อมูลที่กำลังเข้ามาทาง Bluetooth ได้
  static bool wasPressed = false;
  const bool pressed = digitalRead(BTN_PIN) == LOW;
  if (pressed && !wasPressed) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // สลับ LED เอง
  }
  wasPressed = pressed;

  gdd.poll(); // watch() เห็นว่า LED_PIN เปลี่ยน -> ส่ง "LED:.." กลับแอปให้เอง
}
