// ตัวอย่าง ESP32 — โหมด Gamepad
// เปิดไฟล์นี้ตรงใน Arduino IDE ได้เลย (Tools -> Board -> "ESP32 Dev Module")
// แล้วจับคู่กับ "GDD-ESP32" ในแอป ไม่ต้องใช้ PlatformIO
//
// แอปส่งข้อมูลมาได้ 2 แบบ:
//   <B,btn,state>    ปุ่มกด เช่น <B,A,1>  (1=กด 0=ปล่อย)
//   <J,side,x,y>     สติ๊กอนาล็อก เช่น <J,L,0.50,-0.25>

#include <BluetoothSerial.h>
BluetoothSerial BT;
const int LED_PIN = 2;

String field(String f, int n);

void setup() {
  BT.begin("GDD-ESP32");
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (BT.available()) {
    String f = BT.readStringUntil('>');
    if (f.startsWith("<B,")) {
      String btn = field(f, 1);
      int state = field(f, 2).toInt();        // 1=กด 0=ปล่อย
      if (btn == "A") digitalWrite(LED_PIN, state ? HIGH : LOW);
      Serial.println(btn + String(state ? " กด" : " ปล่อย"));
      // เพิ่มปุ่มอื่นที่ต้องการตรงนี้ เช่น if (btn == "B") ...
    } else if (f.startsWith("<J,")) {
      String side = field(f, 1);              // "L" หรือ "R"
      float x = field(f, 2).toFloat();        // -1.00 ถึง 1.00
      float y = field(f, 3).toFloat();
      Serial.println(side + " x=" + String(x) + " y=" + String(y));
      // เพิ่ม logic ขับมอเตอร์ตามค่า x, y ของตัวเองตรงนี้ได้
    }
  }
}

String field(String f, int n) {
  f.replace("<", ""); f.replace(">", "");
  while (n--) f = f.substring(f.indexOf(',') + 1);
  int c = f.indexOf(',');
  return c < 0 ? f : f.substring(0, c);
}
