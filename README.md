# GDDLink

ไลบรารี Arduino เล็กๆ สำหรับรับ-ส่งข้อมูลกับแอป Arduino Bluetooth (GDD) ครบทุกหน้าจอ ไม่ใช่แค่ Switch I/O — Hand, Gamepad, Servo, Car, Tilt, Voice ก็ใช้ได้เหมือนกัน

## คอนเซปต์

อ่านข้อมูลเข้ายังเป็นแบบ Arduino ปกติทุกอย่าง เด็กยังคุม `BT.available()` / `BT.read()` เอง แค่ส่งไบต์ที่อ่านมาให้ `gdd.feed(c)` ทีละตัว ไลบรารีจะแยกแยะเองว่ามาจากหน้าจอไหน แล้วเรียกฟังก์ชันที่ลงทะเบียนไว้ให้

แต่ละหน้าจอมีฟังก์ชันลงทะเบียนของตัวเอง ตั้งแค่ตอน `setup()` แล้วเขียน logic ในฟังก์ชันนั้นได้เลย ไม่ต้องแกะ string เอง:

```cpp
void onGesture(const String &name) {
  if (name == "FIST") digitalWrite(LED, HIGH);
}
void setup() {
  gdd.onGesture(onGesture);   // Hand screen
}
```

## ติดตั้ง

- **Arduino IDE**: คัดลอกโฟลเดอร์ `GDDLink/` ไปไว้ใน `Documents/Arduino/libraries/` หรือ Sketch → Include Library → Add .ZIP Library… แล้วเลือกโฟลเดอร์นี้
- **PlatformIO**: อยู่ใน `firmware/lib/GDDLink/` อยู่แล้ว โปรเจกต์ `firmware/` เห็นเอง แค่ `#include <GDDLink.h>`

## โครงร่างที่ใช้ทุกหน้าจอเหมือนกัน

```cpp
#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);   // อย่าตั้งชื่อตัวแปรว่า "link" เพราะชนกับ libc บน ESP32

void setup() {
  BT.begin("GDD-ESP32");
  // ลงทะเบียนฟังก์ชันของหน้าจอที่ใช้ตรงนี้
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();
}
```

ทุกตัวอย่างใน `examples/` ใช้โครงนี้เหมือนกันหมด ต่างกันแค่ฟังก์ชันที่ลงทะเบียน

## แต่ละหน้าจอ

**Hand** — ท่าทางสำเร็จรูป/ที่ฝึกเอง, แกนต่อเนื่อง, หรือพิกัดดิบ:
```cpp
gdd.onGesture([](const String &name) { ... });               // เช่น "FIST"
gdd.onMotion([](const String &name, int value) { ... });     // แกนฝึกเอง 0-100
gdd.onHand([](int ix, int iy, int tx, int ty, int pinch) { ... });
```

**Gamepad** — ปุ่มกดและสติ๊กอนาล็อก:
```cpp
gdd.onButton([](const String &btn, bool pressed) { ... });
gdd.onStick([](const String &side, float x, float y) { ... }); // side = "L"/"R"
```

**Servo Studio** — channel 1-4, angle 0-180:
```cpp
gdd.onServo([](int channel, int angle) { ... });
```

**Voice** — ข้อความที่แปลงจากเสียงแล้ว:
```cpp
gdd.onVoice([](const String &text) { ... });
```

**Car / Tilt** — ใช้ตัวจบข้อความ `';'` (ดูหัวข้อด้านล่าง) เพราะแอปส่งคำสั่งพวกนี้แบบนั้น:
```cpp
GDDLink gdd(BT, ';');
gdd.onCarSpeed([](int speed) { ... });        // เฉพาะ Car ส่งความเร็วมาด้วย
gdd.onCarCommand([](const String &cmd) { ... }); // "F"/"B"/"L"/"R"/"S" หรือคำสั่งอื่น
```

ดูตัวอย่างเต็มของแต่ละหน้าจอที่ `examples/Hand`, `examples/Gamepad`, `examples/Servo`, `examples/Car`, `examples/Tilt`, `examples/Voice`

## Switch I/O / AI Chat

หน้านี้คุยกันด้วยข้อความรูปแบบ `NAME:value` เช่น `LED:1`, `Temp:25.4` — ผูกชื่อช่องเข้ากับขาตรงๆ ได้เลย ไม่ต้องเขียน callback เอง:

```cpp
String readTemp() { return String(analogRead(A0)); }

void setup() {
  gdd.bindDigitalOut("LED", 2);     // ช่อง LED ในแอป -> ขา 2
  gdd.bindAnalogOut("Motor", 4);    // ช่อง Motor (0-255) -> ขา 4 (PWM)
  gdd.watch("Temp", readTemp);      // อ่านขา แล้วส่งกลับเมื่อค่าเปลี่ยน
}
```

ชื่อช่องสามตัวนี้ (`LED`, `Motor`, `Temp`) ตรงกับค่าตั้งต้นในแอปพอดี ตัวอย่างเต็มอยู่ที่ `examples/SwitchIO/`

`watch()` ไม่ได้ส่งทุก N มิลลิวินาที แต่เช็คค่าได้ไม่เกินทุก `minIntervalMs` (ดีฟอลต์ 200ms) แล้วส่งออกเฉพาะตอนค่าเปลี่ยนจริงเท่านั้น ประหยัดกว่ารูปแบบ timer ส่งทุกวินาทีที่เห็นบ่อยๆ

ชื่อเดียวต้องคุมหลายขา หรือมี logic พิเศษ ใช้ `onReceive(name, callback)` แทน `bindDigitalOut`/`bindAnalogOut` ดู `examples/CustomCallback/`

### รับ-ส่งสองทางบนชื่อเดียวกัน

`bindDigitalOut`/`bindAnalogOut` ส่งทางเดียว (แอป → ขา) ถ้ามีปุ่มจริงบนบอร์ดไปเปลี่ยนขานั้นเอง ไลบรารีไม่รู้ และจะไม่ส่งอะไรกลับไปแอปให้

ผูก `bindDigitalOut` กับ `watch` บนชื่อช่องเดียวกันได้เลย ไม่ชนกัน:

```cpp
gdd.bindDigitalOut("LED", LED_PIN);
gdd.watch("LED", []() { return String(digitalRead(LED_PIN)); });
```

ตัวอย่างเต็ม (ปุ่มกดจริงเปลี่ยน LED แล้วแอปเห็นสถานะอัปเดตทันที) ดู `examples/ButtonSync/`

## API

| ฟังก์ชัน | ทำอะไร |
|---|---|
| `GDDLink gdd(stream, terminator='\n')` | สร้าง instance จาก Stream ใดก็ได้ ปรับตัวจบข้อความได้ |
| `gdd.feed(c)` | ป้อนไบต์ที่อ่านมาทีละตัว แยกเองว่าเป็นข้อความหน้าจอไหนแล้วเรียก callback ที่ตรงกัน |
| `gdd.poll()` | เรียกทุกรอบ loop() เช็คว่า watch() ตัวไหนค่าเปลี่ยนแล้วส่งออก |
| `gdd.send(name, value)` | ส่ง `name:value<terminator>` ทันที (รับ String/int/long/float) |
| `gdd.onGesture(fn)` | Hand: ท่าทางสำเร็จรูป/ที่ฝึกเอง |
| `gdd.onMotion(fn)` | Hand: แกนต่อเนื่องที่ฝึกเอง |
| `gdd.onHand(fn)` | Hand: พิกัดดิบ 5 ค่า |
| `gdd.onButton(fn)` | Gamepad: ปุ่มกด |
| `gdd.onStick(fn)` | Gamepad: สติ๊กอนาล็อก |
| `gdd.onServo(fn)` | Servo Studio: channel + angle |
| `gdd.onVoice(fn)` | Voice: ข้อความที่แปลงจากเสียง |
| `gdd.onCarSpeed(fn)` | Car: ความเร็ว 0-255 |
| `gdd.onCarCommand(fn)` | Car/Tilt: คำสั่ง F/B/L/R/S หรืออื่นๆ |
| `gdd.bindDigitalOut(name, pin)` | Switch I/O: ผูกชื่อช่องกับขา digital |
| `gdd.bindAnalogOut(name, pin)` | Switch I/O: ผูกชื่อช่องกับขา PWM |
| `gdd.onReceive(name, fn)` | Switch I/O: เรียก fn(value) ทุกครั้งที่ได้รับชื่อนี้ |
| `gdd.watch(name, fn, minIntervalMs=200)` | Switch I/O: เช็ค fn() แล้วส่งออกเฉพาะตอนค่าเปลี่ยน |
| `gdd.value(name)` / `gdd.valueInt(name)` | Switch I/O: ค่าล่าสุดที่ได้รับของชื่อนี้ |

## หน่วยความจำ

ไม่ใช้ heap เลย ช่อง Switch I/O เก็บใน array ขนาดคงที่ตอนคอมไพล์ ดีฟอลต์ 16 ช่อง ปลอดภัยกับบอร์ดเล็กอย่าง Uno/Nano ที่มี RAM แค่ 2KB (callback ของหน้าจออื่นเก็บเป็น function pointer ตัวเดียวต่อหน้าจอ ไม่ใช้ array เลย)

ถ้าต้องการ Switch I/O มากกว่า 16 ช่อง ใส่บรรทัดนี้ก่อน `#include <GDDLink.h>`:
```cpp
#define GDDLINK_MAX_CHANNELS 24
#include <GDDLink.h>
```

## ตัวจบข้อความ (terminator)

Hand/Gamepad/Servo/Voice ใช้กรอบ `<...>` อยู่แล้ว ไม่ต้องสนใจตัวจบข้อความ ส่วน Switch I/O กับ Car/Tilt ไม่มีกรอบ เลยต้องมีตัวจบบอกว่าข้อความหนึ่งจบตรงไหน

ดีฟอลต์คือ `\n` ใช้ได้เลยกับ Switch I/O เดี่ยวๆ แต่ถ้าใช้ Car หรือ Tilt (หรือใช้ร่วมกับ Switch I/O บนลิงก์เดียวกัน) เปลี่ยนเป็น `';'` ที่ constructor เพราะแอปส่งคำสั่งพวกนี้แบบนั้น:

```cpp
GDDLink gdd(BT, ';');
```

ดูตัวอย่างจริงใน [`firmware/src/main.cpp`](https://github.com/Ninlapat5G/Arduino-Bluetooth-Mobile/blob/v2-ai-chat/firmware/src/main.cpp) ของแอป Arduino Bluetooth (GDD)

## ตัวอย่างทั้งหมด

| ตัวอย่าง | โหมดในแอป | สาธิตอะไร |
|---|---|---|
| `examples/SwitchIO/` | Switch I/O | ผูกชื่อช่องเข้ากับขาตรงๆ ตรงกับช่องตั้งต้นของแอป |
| `examples/CustomCallback/` | Switch I/O | `onReceive()` เขียน logic เอง |
| `examples/ButtonSync/` | Switch I/O | สองทางบนชื่อช่องเดียวกัน |
| `examples/Hand/` | Hand | ท่าทาง/พิกัดมือ ขยับ servo 2 แกน |
| `examples/Gamepad/` | Gamepad | ปุ่มกด + สติ๊กอนาล็อก |
| `examples/Servo/` | Servo Studio | สั่ง servo สูงสุด 4 ตัว |
| `examples/Car/` | Car Controller | ขับมอเตอร์ผ่าน L298N |
| `examples/Tilt/` | Tilt Control | ขับรถแบบเอียงมือถือ ใช้มอเตอร์ชุดเดียวกับ Car |
| `examples/Voice/` | Voice | หาคำสำคัญจากข้อความที่แปลงจากเสียง |

## ทดสอบบนฮาร์ดแวร์จริง

ผ่านการทดสอบจริงบน ESP32 WROOM (build ด้วย PlatformIO, flash จริง, ยิงคำสั่งผ่าน Serial แล้วตรวจขาจริงว่าเปลี่ยนค่าตามที่คาด) ไม่ใช่แค่รีวิวโค้ด รวมถึงการ dispatch ของ callback ใหม่ทั้ง 9 ตัว (Hand/Gamepad/Servo/Voice/Car/Tilt) ที่ทดสอบยิง byte ตรงรูปแบบเดียวกับที่แอปส่งจริง

## License

Apache License 2.0 — ดูไฟล์ [LICENSE](LICENSE)

---

สร้างโดย **ครูนิน × Claude**
