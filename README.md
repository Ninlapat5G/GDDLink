# GDDLink

ไลบรารี Arduino ที่เล็กที่สุดเท่าที่จะทำได้ สำหรับ **รับ-ส่งข้อมูล** กับแอป [Arduino Bluetooth (GDD)](https://github.com/Ninlapat5G/Arduino-Bluetooth-Mobile) เท่านั้น — ไม่ยุ่งกับ gesture/gamepad/car ให้เพิ่มความซับซ้อน

แอป (หน้า Switch I/O และ AI Chat) คุยกันด้วยข้อความบรรทัดเดียวรูปแบบ `NAME:value` (เช่น `LED:1`, `Temp:25.4`) — ไลบรารีนี้ทำหน้าที่แค่ "แปลงบรรทัดนั้นให้เป็นขาจริง" และ "ส่งค่ากลับ" ให้ง่ายที่สุด

**การอ่านข้อมูลเข้ายังเป็นแบบ Arduino มาตรฐาน** — เด็กยังคุม `if/while (BT.available())` กับ `BT.read()` เองเหมือนตัวอย่าง .ino อื่นๆ ในแอป ไลบรารีนี้ไม่แย่งงานอ่านไปทำเอง รับแค่ "ตัวอักษรที่อ่านมาแล้ว" ผ่าน `gdd.feed(c)` ไปจัดรูปให้เป็น `NAME:value`

## ติดตั้ง

- **Arduino IDE**: ดาวน์โหลด (Code → Download ZIP) แล้ว Sketch → Include Library → Add .ZIP Library… เลือกไฟล์ที่โหลดมา หรือแตกซิปแล้วคัดลอกทั้งโฟลเดอร์ไปไว้ใน `Documents/Arduino/libraries/GDDLink/`
- **PlatformIO**: เพิ่มใน `platformio.ini` ของโปรเจกต์
  ```ini
  lib_deps = https://github.com/Ninlapat5G/GDDLink.git
  ```

## ใช้งานแบบง่ายที่สุด (ผูกชื่อ → ขา โดยตรง)

```cpp
#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);           // ใช้กับ Stream อะไรก็ได้: BluetoothSerial, Serial, Serial1, SoftwareSerial
                            // (อย่าตั้งชื่อตัวแปรว่า "link" — ชนกับ libc บน ESP32)

String readTemp() { return String(analogRead(A0)); }

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindDigitalOut("LED", 2);      // ช่อง "LED" (Digital) ในแอป -> ขา 2
  gdd.bindAnalogOut("Motor", 4);     // ช่อง "Motor" (Analog 0-255) ในแอป -> ขา 4 (PWM)
  gdd.watch("Temp", readTemp);       // อ่านขา -> ส่งกลับไปแอป "เฉพาะตอนค่าเปลี่ยน"
}

void loop() {
  while (BT.available()) {
    gdd.feed(BT.read());             // อ่านเองทีละไบต์ เหมือนตัวอย่างอื่นๆ ในแอป
  }
  gdd.poll();                        // เช็ค watch() — ส่ง "Temp:.." ทันทีที่ค่าเปลี่ยน
}
```

สามตัวอย่างนี้ตรงกับช่องตั้งต้นของแอป (`LED`, `Motor`, `Temp`) พอดี — ดูตัวอย่างเต็มที่ `examples/SwitchIO/`

**หมายเหตุ:** `watch()` ตั้งใจไม่ส่งแบบ "ทุก N มิลลิวินาที" — จะเช็คค่าได้ไม่เกินทุก `minIntervalMs` (ดีฟอลต์ 200ms กันอ่านถี่เกินไป) แต่จะ **ส่งออก BT ก็ต่อเมื่อค่าจริงๆ เปลี่ยน** เท่านั้น ประหยัดแบนด์วิดท์/พลังงานกว่ารูปแบบ timer ส่งทุกวินาทีที่เห็นใน sketch ตัวอย่างทั่วไป

## ถ้าต้องการ logic เอง

ใช้ `onReceive(name, callback)` แทน `bindDigitalOut`/`bindAnalogOut` เมื่อชื่อเดียวต้องคุมหลายขา หรือมี logic พิเศษ — ดู `examples/CustomCallback/`

## รับ-ส่งสองทางพร้อมกัน บนชื่อช่องเดียวกัน

`bindDigitalOut`/`bindAnalogOut` เป็นแค่ทิศทางเดียว (แอป → ขา) — ถ้ามีอะไรไปเปลี่ยนขานั้นเอง เช่นปุ่มกดจริงบนบอร์ด ตัวไลบรารีจะไม่รู้ และจะไม่ส่งอะไรกลับไปแอปให้เองอัตโนมัติ

ถ้าอยากให้แอปเห็นสถานะจริงของขาด้วย (ไม่ว่าใครเป็นคนเปลี่ยน) ผูก `bindDigitalOut` กับ `watch` บน **ชื่อช่องเดียวกัน** ได้เลย ไม่ชนกัน:

```cpp
gdd.bindDigitalOut("LED", LED_PIN);                          // แอป -> ขา
gdd.watch("LED", []() { return String(digitalRead(LED_PIN)); }); // ขา -> แอป
```

ตัวอย่างเต็มที่ปุ่มกดจริงเปลี่ยน LED แล้วแอปเห็นสถานะอัปเดตทันที ดู `examples/ButtonSync/`

## อ้างอิง API

| ฟังก์ชัน | ทำอะไร |
|---|---|
| `GDDLink gdd(stream)` | สร้าง instance จาก `Stream` ใดก็ได้ (ตั้งชื่อตัวแปรอะไรก็ได้ แต่อย่าใช้ `link` — ชนกับ libc บน ESP32) |
| `gdd.feed(c)` | ส่งตัวอักษรที่อ่านมาเอง (`BT.read()`) เข้าไปทีละตัว — สะสมจนครบบรรทัด แล้ว parse `NAME:value`, สั่งขาที่ bind ไว้, เรียก callback ให้ |
| `gdd.poll()` | เรียกทุกรอบ `loop()` (ไม่ต้องมีเงื่อนไข) — เช็คว่า `watch()` ตัวไหนค่าเปลี่ยนแล้วส่งออก ไม่เกี่ยวกับการอ่านข้อมูลเข้า |
| `gdd.send(name, value)` | ส่ง `name:value\n` ออกไปทันที (รับ `String`/`int`/`long`/`float`) |
| `gdd.bindDigitalOut(name, pin)` | ผูกชื่อช่องกับขา digital (`pinMode(OUTPUT)` ให้อัตโนมัติ) |
| `gdd.bindAnalogOut(name, pin)` | ผูกชื่อช่องกับขา PWM (0-255) |
| `gdd.onReceive(name, fn)` | เรียก `fn(value)` ทุกครั้งที่ได้รับชื่อนี้ |
| `gdd.watch(name, fn, minIntervalMs=200)` | เช็ค `fn()` อย่างเร็วสุดทุก `minIntervalMs` แล้วส่งออกเฉพาะตอนค่าเปลี่ยนจากที่ส่งล่าสุด |
| `gdd.value(name)` / `gdd.valueInt(name)` | ค่าล่าสุดที่ได้รับของชื่อนี้ |

## หน่วยความจำ — สไตล์ Arduino จริงๆ ไม่ใช้ heap

จำนวนช่อง (channel) สูงสุดที่ track ได้ถูกกำหนดตอนคอมไพล์ ดีฟอลต์ **16 ช่อง** เก็บใน fixed array ไม่มีการ `new`/`malloc` ระหว่างทำงานเลย — ปลอดภัยกับบอร์ดเล็กอย่าง Uno/Nano ที่มี RAM แค่ 2KB (heap fragmentation จากการจองหน่วยความจำขณะรันเป็นปัญหาจริงบนบอร์ดพวกนี้)

ถ้าต้องการมากกว่า 16 ช่อง ใส่บรรทัดนี้ **ก่อน** `#include <GDDLink.h>`:
```cpp
#define GDDLINK_MAX_CHANNELS 24
#include <GDDLink.h>
```

## ตัวอย่างทั้งหมด

| ตัวอย่าง | สาธิตอะไร |
|---|---|
| `examples/SwitchIO/` | ผูกชื่อช่องเข้ากับขาโดยตรง ตรงกับช่องตั้งต้นของแอป (LED/Motor/Temp) |
| `examples/CustomCallback/` | ใช้ `onReceive()` เขียน logic เองเมื่อ bind ตรงๆ ไม่พอ |
| `examples/ButtonSync/` | ผูกชื่อเดียวกันสองทาง — ปุ่มกดจริงเปลี่ยนค่า แล้วแอปเห็นสถานะอัปเดตอัตโนมัติ |

## ทดสอบบนฮาร์ดแวร์จริง

ตัวไลบรารีนี้ผ่านการทดสอบจริงบน ESP32 WROOM (build ด้วย PlatformIO, flash จริง, ยิงคำสั่งผ่าน Serial และตรวจสอบว่าขาจริงเปลี่ยนค่าตามที่คาด) — ไม่ใช่แค่รีวิวโค้ดอย่างเดียว

## License

Apache License 2.0 — ดูไฟล์ [LICENSE](LICENSE)
