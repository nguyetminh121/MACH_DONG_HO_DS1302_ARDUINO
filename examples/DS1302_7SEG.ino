#include <RtcDS1302.h>

// Kết nối với 74HC595
int latchPin = 8;   // ST_CP
int clockPin = 12;  // SH_CP
int dataPin = 11;   // DS

// Kết nối các button
int buttonPowerPin = 7;  // Nút nhấn tắt/mở nguồn
int buttonDisplayPin = 6;  // Nút nhấn chuyển đổi hiển thị

int ledPin = 9;

bool systemOn = true; 
bool lastButtonPowerState = HIGH;
bool lastButtonDisplayState = HIGH; /

bool displayTime = true; // Trạng thái hiển thị, true: hiển thị giờ/phút, false: hiển thị ngày/tháng

unsigned long lastUpdateTime = 0; 
unsigned long lastButtonCheckTime = 0; // Thời gian kiểm tra nút nhấn

const unsigned long updateInterval = 10000; 
const unsigned long buttonCheckInterval = 100;

// Các đoạn LED tương ứng với số từ 0-9
const int Seg[10] = {
  0b11000000, //0
  0b11111001, //1
  0b10100100, //2
  0b10110000, //3
  0b10011001, //4
  0b10010010, //5
  0b10000011, //6
  0b11111000, //7
  0b10000000, //8
  0b10010000  //9
};

// Kết nối DS1302
ThreeWire myWire(3, 4, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(buttonPowerPin, INPUT_PULLUP);
  pinMode(buttonDisplayPin, INPUT_PULLUP); 

  pinMode(ledPin, OUTPUT); 

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Kiểm tra trạng thái của nút nguồn mỗi 100 ms
  if (currentMillis - lastButtonCheckTime >= buttonCheckInterval) {
    lastButtonCheckTime = currentMillis;

    bool currentButtonPowerState = digitalRead(buttonPowerPin);
    if (currentButtonPowerState == LOW && lastButtonPowerState == HIGH) {
      delay(50); 
      if (digitalRead(buttonPowerPin) == LOW) {
        systemOn = !systemOn; 
        Serial.println(systemOn ? "System ON" : "System OFF");
        delay(300); 
      }
    }
    lastButtonPowerState = currentButtonPowerState;

    bool currentButtonDisplayState = digitalRead(buttonDisplayPin);
    if (currentButtonDisplayState == LOW && lastButtonDisplayState == HIGH) {
      delay(50); 
      if (digitalRead(buttonDisplayPin) == LOW) {
        displayTime = !displayTime; 
        Serial.println(displayTime ? "Display Time" : "Display Date");

        digitalWrite(ledPin, displayTime ? LOW : HIGH); 

        delay(300); 
      }
    }
    lastButtonDisplayState = currentButtonDisplayState;
  }

  if (currentMillis - lastUpdateTime >= updateInterval && systemOn) {
    lastUpdateTime = currentMillis;

    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if (displayTime) {
      // Hiển thị giờ/phút lên LED 7 đoạn
      int hour = now.Hour();
      int minute = now.Minute();

      displayNumber(hour / 10);   // Hàng chục của giờ
      delay(5);
      displayNumber(hour % 10);   // Hàng đơn vị của giờ
      delay(5);

      displayNumber(minute / 10); // Hàng chục của phút
      delay(5);
      displayNumber(minute % 10); // Hàng đơn vị của phút
      delay(5);
    } else {
      // Hiển thị ngày/tháng lên LED 7 đoạn
      int day = now.Day();
      int month = now.Month();

      displayNumber(day / 10);   // Hàng chục của ngày
      delay(5);
      displayNumber(day % 10);   // Hàng đơn vị của ngày
      delay(5);

      displayNumber(month / 10); // Hàng chục của tháng
      delay(5);
      displayNumber(month % 10); // Hàng đơn vị của tháng
      delay(5);
    }
  }

  if (!systemOn) {
    // Nếu hệ thống tắt, tắt tất cả các LED
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 0xFF);
    digitalWrite(latchPin, HIGH);
  }
}

// Hàm hiển thị số trên LED 7 đoạn
void displayNumber(int num) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, Seg[num]);
  digitalWrite(latchPin, HIGH);
}

// Hàm hiển thị thời gian lên Serial Monitor
void printDateTime(const RtcDateTime& dt) {
  char datestring[20];
  snprintf_P(datestring, 
             sizeof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}
