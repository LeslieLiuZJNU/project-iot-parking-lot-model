#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8glib.h>

#define CAR_UP 90
#define CAR_DOWN 180
#define DOOR_UP 180
#define DOOR_DOWN 90

int mode = 1;
int SS_PIN[2] = {5, 7};
int RST_PIN[2] = {4, 6};

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);

Servo s_door;
Servo s_car;

SoftwareSerial bt(10, 11);

MFRC522 rfid_wrong(SS_PIN[0], RST_PIN[0]);
MFRC522 rfid_right(SS_PIN[1], RST_PIN[1]);
MFRC522 rfids[2] = {rfid_wrong, rfid_right};

//Store NUID
byte nuidPICC[4];

void setup() {
  screenInit();
  screenShow(2);
  s_door.attach(8);
  s_door.write(DOOR_UP);
  s_car.attach(9);
  s_car.write(CAR_DOWN);
  bt.begin(9600);
  Serial.begin(9600);
  SPI.begin(); //Init SPI bus
  for (int i = 0; i <= 1; i++)
    rfids[i].PCD_Init(); //Init RC522
}

void loop() {
  switch (mode) {
    case 1:
      checkAppointment();
      break;
    case 2:
      checkArrival();
      break;
    case 3:
      scanIC();
      break;
    case 4:
      waitForCorrectPosition();
      break;
  }
}

void screenInit() {
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }
}

void screenShow(int i) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_unifont);
    if (i == 0)
      u8g.drawStr( 0, 22, "Free Space: 0");
    else if (i == 1)
      u8g.drawStr( 0, 22, "Free Space: 1");
    else if (i == 2)
      u8g.drawStr( 0, 22, "Free Space: 2");
  } while ( u8g.nextPage() );
}

void checkAppointment() {
  if (bt.available()) {
    int in = bt.read();
    if (in == 2) {
      Serial.print(in);
      bt.print("2");
      mode = 2;
      s_car.write(CAR_UP);
      screenShow(1);
    }

  }
}

void checkArrival() {
  if (bt.available()) {
    int in = bt.read();
    if (in == 3) {
      bt.print("3");
      Serial.print("3");
      mode = 3;
      s_door.write(DOOR_DOWN);
      s_car.write(CAR_DOWN);
    }
  }
}

void scanIC() {
  for (int i = 0; i <= 1; i++) {

    for (int j = 0; j <= 1; j++) {
      if (j == i)digitalWrite(SS_PIN[j], 0);
      else digitalWrite(SS_PIN[j], 1);
    }

    //Look for card
    if (!rfids[i].PICC_IsNewCardPresent())
      continue;

    //Try if NUID is readable
    if (!rfids[i].PICC_ReadCardSerial())
      continue;

    if (i == 0) {
      mode = 4;
      bt.print("4");
      Serial.print("4");
      screenShow(0);
    }
    else if (i == 1) {
      mode = 5;
      bt.print("5");
      Serial.print("5");
    }

    //Let the holding card be in sleep mode, make sure to read once
    rfids[i].PICC_HaltA();

    //Stop reading
    rfids[i].PCD_StopCrypto1();
  }
}

void waitForCorrectPosition() {
  if (bt.available()) {
    int in = bt.read();
    if (in == 6) {
      bt.print("6");
      Serial.print("6");
      mode = 6;
      s_door.write(DOOR_DOWN);
      screenShow(1);
    }
  }
  int i = 1;
  for (int j = 0; j <= 1; j++) {
    if (j == i)digitalWrite(SS_PIN[j], 0);
    else digitalWrite(SS_PIN[j], 1);
  }

  //Look for card
  if (!rfids[i].PICC_IsNewCardPresent())
    return;

  //Try if NUID is readable
  if (!rfids[i].PICC_ReadCardSerial())
    return;

  mode = 5;
  bt.print("5");
  Serial.print("5");
  screenShow(1);

  //Let the holding card be in sleep mode, make sure to read once
  rfids[i].PICC_HaltA();

  //Stop reading
  rfids[i].PCD_StopCrypto1();
}
