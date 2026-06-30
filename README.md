# GDDLink

ไลบรารี Arduino เล็กๆ สำหรับรับ-ส่งข้อมูลกับแอป Arduino Bluetooth (GDD) เฉพาะหน้า Switch I/O และ AI Chat เท่านั้น ไม่ยุ่งกับ Hand, Gamepad, Car เพื่อให้ใช้ง่ายจริงๆ

## คอนเซปต์

แอปคุยกับบอร์ดด้วยข้อความบรรทัดเดียวรูปแบบ `NAME:value` เช่น `LED:1`, `Temp:25.4` GDDLink ทำแค่สองอย่าง: แปลงบรรทัดที่อ่านมาให้เป็นการสั่งขาจริง และส่งค่ากลับไปแอปเวลาขาเปลี่ยน

การอ่านข้อมูลเข้ายังเป็นแบบ Arduino ปกติทุกอย่าง เด็กยังคุม `BT.available()` / `BT.read()` เองเหมือนตัวอย่างอื่นในแอป ไลบรารีนี้รับแค่ตัวอักษรที่อ่านมาแล้วผ่าน `gdd.feed(c)` ไปจัดเป็น `NAME:value` ให้

## ติดตั้ง

- **Arduino IDE**: คัดลอกโฟลเดอร์ `GDDLink/` ไปไว้ใน `Documents/Arduino/libraries/` หรือ Sketch → Include Library → Add .ZIP Library… แล้วเลือกโฟลเดอร์นี้
- **PlatformIO**: อยู่ใน `firmware/lib/GDDLink/` อยู่แล้ว โปรเจกต์ `firmware/` เห็นเอง แค่ `#include <GDDLink.h>`

## ใช้งานพื้นฐาน

```cpp
#include <BluetoothSerial.h>
#include <GDDLink.h>

BluetoothSerial BT;
GDDLink gdd(BT);   // ใช้กับ Stream อะไรก็ได้ เช่น Serial, SoftwareSerial
                    // อย่าตั้งชื่อตัวแปรว่า "link" เพราะชนกับ libc บน ESP32

String readTemp() { return String(analogRead(A0)); }

void setup() {
  BT.begin("GDD-ESP32");
  gdd.bindDigitalOut("LED", 2);     // ช่อง LED ในแอป -> ขา 2
  gdd.bindAnalogOut("Motor", 4);    // ช่อง Motor (0-255) -> ขา 4 (PWM)
  gdd.watch("Temp", readTemp);      // อ่านขา แล้วส่งกลับเมื่อค่าเปลี่ยน
}

void loop() {
  while (BT.available()) gdd.feed(BT.read());
  gdd.poll();   // ส่งค่า watch() ที่เปลี่ยนกลับไปแอป
}
```

ชื่อช่องสามตัวนี้ (`LED`, `Motor`, `Temp`) ตรงกับค่าตั้งต้นในแอปพอดี ตัวอย่างเต็มอยู่ที่ `examples/SwitchIO/`

`watch()` ไม่ได้ส่งทุก N มิลลิวินาที แต่เช็คค่าได้ไม่เกินทุก `minIntervalMs` (ดีฟอลต์ 200ms) แล้วส่งออกเฉพาะตอนค่าเปลี่ยนจริงเท่านั้น ประหยัดกว่ารูปแบบ timer ส่งทุกวินาทีที่เห็นบ่อยๆ

## เขียน logic เอง

ชื่อเดียวต้องคุมหลายขา หรือมี logic พิเศษ ใช้ `onReceive(name, callback)` แทน `bindDigitalOut`/`bindAnalogOut` ดู `examples/CustomCallback/`

## รับ-ส่งสองทางบนชื่อเดียวกัน

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
| `gdd.feed(c)` | ป้อนตัวอักษรที่อ่านมาทีละตัว สะสมจนครบบรรทัดแล้ว parse เป็น `NAME:value` |
| `gdd.poll()` | เรียกทุกรอบ loop() เช็คว่า watch() ตัวไหนค่าเปลี่ยนแล้วส่งออก |
| `gdd.send(name, value)` | ส่ง `name:value<terminator>` ทันที (รับ String/int/long/float) |
| `gdd.bindDigitalOut(name, pin)` | ผูกชื่อช่องกับขา digital |
| `gdd.bindAnalogOut(name, pin)` | ผูกชื่อช่องกับขา PWM |
| `gdd.onReceive(name, fn)` | เรียก fn(value) ทุกครั้งที่ได้รับชื่อนี้ |
| `gdd.watch(name, fn, minIntervalMs=200)` | เช็ค fn() แล้วส่งออกเฉพาะตอนค่าเปลี่ยน |
| `gdd.value(name)` / `gdd.valueInt(name)` | ค่าล่าสุดที่ได้รับของชื่อนี้ |

## หน่วยความจำ

ไม่ใช้ heap เลย ช่องสัญญาณเก็บใน array ขนาดคงที่ตอนคอมไพล์ ดีฟอลต์ 16 ช่อง ปลอดภัยกับบอร์ดเล็กอย่าง Uno/Nano ที่มี RAM แค่ 2KB

ถ้าต้องการมากกว่า 16 ช่อง ใส่บรรทัดนี้ก่อน `#include <GDDLink.h>`:
```cpp
#define GDDLINK_MAX_CHANNELS 24
#include <GDDLink.h>
```

## ตัวจบข้อความ (terminator)

ดีฟอลต์คือ `\n` ถ้าลิงก์เดียวกันต้องใช้ร่วมกับโปรโตคอลอื่นที่ไม่มีตัวจบของตัวเอง เช่นคำสั่งรถ `F`/`B`/`L`/`R` ตัวอักษรเดียว เปลี่ยนตัวจบได้ที่ constructor:

```cpp
GDDLink gdd(BT, ';');
```

ดูตัวอย่างจริงใน [`firmware/src/main.cpp`](https://github.com/Ninlapat5G/Arduino-Bluetooth-Mobile/blob/v2-ai-chat/firmware/src/main.cpp) ของแอป Arduino Bluetooth (GDD)

## ตัวอย่างทั้งหมด

| ตัวอย่าง | สาธิตอะไร |
|---|---|
| `examples/SwitchIO/` | ผูกชื่อช่องเข้ากับขาตรงๆ ตรงกับช่องตั้งต้นของแอป |
| `examples/CustomCallback/` | `onReceive()` เขียน logic เอง |
| `examples/ButtonSync/` | สองทางบนชื่อช่องเดียวกัน |

โหมดอื่นของแอป (Hand, Gamepad, Servo, Car, Tilt, Voice) ไม่เกี่ยวกับ GDDLink — แต่ละโหมดมีตัวอย่างของตัวเองที่ [`firmware/examples/`](https://github.com/Ninlapat5G/Arduino-Bluetooth-Mobile/tree/v2-ai-chat/firmware/examples) ในแอป

## ทดสอบบนฮาร์ดแวร์จริง

ผ่านการทดสอบจริงบน ESP32 WROOM (build ด้วย PlatformIO, flash จริง, ยิงคำสั่งผ่าน Serial แล้วตรวจขาจริงว่าเปลี่ยนค่าตามที่คาด) ไม่ใช่แค่รีวิวโค้ด

## License

Apache License 2.0 — ดูไฟล์ [LICENSE](LICENSE)

---

สร้างโดย **ครูนิน × Claude**
