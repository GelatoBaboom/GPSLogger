
#include <OneWire.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include "FreeSerifBoldItalic18pt7b.h"
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

#include <SPI.h>
#include <SD.h>

TinyGPSPlus gps;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ONE_WIRE_BUS D3
#define CS_PIN  D8

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
File fi;
bool regEnable = false;
char temperatureString[6];
const int led = 13;
String fileName = "";
int timeZone = -3;
bool initilalized = false;
int buttonPin = D4;
uint32_t timerData;

int currentTemp = 0;
float getTemperature() {
  float temp;

  do {
    //Serial.println(F("get temp"));
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    //Serial.println(String((temp)));
    //Serial.println(String((int)trunc(round(temp))));
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));

  return temp;
}

void registerData()
{
  if (regEnable) {
    File fiReg;

    int currentMonth = gps.date.month();
    int currentDay = gps.date.day();
    int currentYear = gps.date.year();
    if (fileName == "") {
      fileName = (localizeHourTime(gps.time.hour()) < 10 ? "route_0" : "route_") + String(localizeHourTime(gps.time.hour())) + (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute())  + (gps.time.second() < 10 ? "0" : "") + String(gps.time.second()) + ".gpx";
    }
    if (currentYear > 2000) {
      if (!SD.exists("/regs/" + String(currentYear) + "/" + String(currentMonth) + "/" + String(currentDay)))
      {
        SD.mkdir("/regs/" + String(currentYear) + "/" + String(currentMonth) + "/" + String(currentDay));
      }
      String datetime = String(gps.date.year())  + "-" + (gps.date.month() < 10 ? "0" : "") + String(gps.date.month()) + "-" + ((gps.date.day() < 10 ? "0" : "") + String(gps.date.day())) + "T" + (localizeHourTime(gps.time.hour()) < 10 ? "0" : "") + String(localizeHourTime(gps.time.hour())) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "Z" ;
      if (!SD.exists("/regs/" + String(currentYear) + "/" + String(currentMonth) + "/" + String(currentDay) + "/" + fileName))
      {
        fiReg = SD.open("/regs/" + String(currentYear) + "/" + String(currentMonth) + "/" + String(currentDay) + "/" + fileName, FILE_WRITE);
        if (fiReg) {
          if (gps.altitude.isValid()) {
            fiReg.println(  "type,latitude,longitude,altitude (m),time,name,desc,speed (km/h)" );
            fiReg.println( "T," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," + String(gps.altitude.meters())  + "," + datetime + ",start," + String(currentTemp) + "," + String(gps.speed.kmph()) );
            fiReg.close();
          }
        }
      } else {
        if (gps.altitude.isValid()) {
          fiReg = SD.open("/regs/" + String(currentYear) + "/" + String(currentMonth) + "/" + String(currentDay) + "/" + fileName, FILE_WRITE);
          if (fiReg) {
            fiReg.println( "T," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," + String(gps.altitude.meters()) + "," + datetime + ",," + String(currentTemp) + "," + String(gps.speed.kmph()));
            fiReg.close();
          }
        }
      }
    }
  }
}
void setup(void) {
  Serial.begin(9600);
  Serial.println(F("init"));
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println(F("init ok"));

  // set the data rate for the SoftwareSerial port
  Serial.print("Starting...");

  pinMode(buttonPin, INPUT);

  display.display();
  display.clearDisplay();
  display.print("test");
  display.display();

  if (!SD.begin(CS_PIN)) {

    Serial.println("initialization failed!");
    display.setCursor(1, 1);
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.print("Error SD");
    display.display();
    while (1) {
      delay(80);
    }
  }
}
void initializingGPS() {
  display.clearDisplay();
  display.drawCircle(20, 30, 15, 1);
  display.fillCircle(23, 28, 1, 1);
  display.fillRoundRect(13, 25, 10, 10, 3, 1);
  display.fillTriangle(14, 30, 20, 30, 15, 40, 1);
  display.fillTriangle(10, 19, 12, 26, 12, 22, 1);
  display.fillTriangle(31, 30, 33, 25, 33, 35, 1);
  display.drawLine(34, 33, 34, 27, 1);
  display.drawLine(15, 41, 16, 42, 1);
  display.drawLine(18, 42, 18, 42, 1);
  display.setCursor(45, 30);
  display.setFont(&FreeSerifBoldItalic18pt7b);
  display.setTextColor(WHITE);
  display.print("GPS");
  display.setCursor(30, 55);
  display.setFont(NULL);
  display.print("Acquiring signal");
  display.display();
}
int localizeHourTime(int hours) {
  hours = hours + timeZone;
  hours = hours < 0 ? hours + 24 : hours;
  hours = hours > 23 ? hours - 24 : hours;
  return hours;
}
void gpsdata()
{
  bool done = false;
  Serial.println("gps try..");
  int tries = 0;
  if (!initilalized) {
    initializingGPS();
  }
  while (!done) {
    currentTemp = (int)trunc(round(getTemperature()));
    smartdelay(500);
    
    Serial.println("DONE-----------------------------------");
    Serial.println("gps data");
    Serial.println((gps.date.day() < 10 ? "0" : "") + String(gps.date.day()) + "/" + (gps.date.month() < 10 ? "0" : "") + String(gps.date.month()) + "/" + String(gps.date.year()) + " - " + (localizeHourTime(gps.time.hour()) < 10 ? "0" : "") + String(localizeHourTime(gps.time.hour())) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second())  );
    Serial.println("lat: " + String(gps.location.lat(), 6) + " long: " + String(gps.location.lng(), 6) );
    Serial.println("vel: "  + String(gps.speed.kmph()) + "Km/h");
    Serial.println("alt: "  + String(gps.altitude.meters()) + "mts");
    Serial.println("sat's: "  + String(gps.satellites.value()));
    Serial.println("pres: "  + String(gps.hdop.value()) + "hdop");
    Serial.println("temp: "  + String(currentTemp) );
    Serial.println("time to get data: "  + String((millis() - timerData) / 1000));
    timerData = millis();
    tries++;
    
    if (gps.altitude.isValid() || tries > 100) {


      done = true;
      initilalized = true;


      display.clearDisplay();
      display.setTextSize(1);

      display.setTextColor(WHITE);
      display.setCursor(1, 1);
      display.print((gps.date.day() < 10 ? "0" : "") + String(gps.date.day()) + "/" + (gps.date.month() < 10 ? "0" : "") + String(gps.date.month()) + "/" + String(gps.date.year()) + " - " + (localizeHourTime(gps.time.hour()) < 10 ? "0" : "") + String(localizeHourTime(gps.time.hour())) + ":" + (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + ":" + (gps.time.second() < 10 ? "0" : "") + String(gps.time.second())  );

      display.setCursor(1, 10);
      display.print(String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6));

      display.setCursor(1, 20);
      display.print("alt: "  + String(gps.altitude.meters()) + "mts");

      display.setCursor(89, 20);
      display.print("t:" + String(currentTemp) + "c");

      display.setCursor(1, 30);
      display.print("sat's:"  + String(gps.satellites.value()));

      display.setCursor(55, 30);
      display.print("v:"  + String(gps.speed.kmph()) + "Kmh");
      display.display();

      if (regEnable) {

        display.setCursor(1, 40);
        int buttonState = 0;
        buttonState = digitalRead(buttonPin);
        if (fileName != "" && buttonState == LOW) {
          display.println("Keep pressing to     create new file");
          display.display();
          smartdelay(1000);
          buttonState = digitalRead(buttonPin);
          if (buttonState == LOW ) {
            fileName = "";
            display.fillRect(0, 40, 128, 20, 0);
            display.display();
            display.setCursor(1, 40);
            display.print("file: route_" + String(gps.time.hour() + timeZone) +  String(gps.time.minute())  + String(gps.time.second()) + ".gpx" ) ;
            display.display();
            smartdelay(1000);
          }

          display.fillRect(0, 40, 128, 20, 0);
        }
        display.setCursor(1, 40);
        display.print("Recording");
        bool flash = true;
        for (int i = 0; i < 4; i++) {
          display.fillCircle(62, 44, 4, flash ? 1 : 0);
          flash = !flash;
          display.display();
          smartdelay(500);
        }
        registerData();
      } else {
        smartdelay(2000);
      }
      //display.print("t.");
      //display.print(String((int)trunc(round(getTemperature()))));

      while (Serial.available() > 0)
      {
        Serial.read();
      }

    }

  }


}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}

void loop(void) {
  //      Serial.println("GPS stream dump:");
  //      while (true) // infinite loop
  //        if (Serial.available() > 0) // any data coming in?
  //          Serial.write(Serial.read());

  // check if the pushbutton is pressed.

  int buttonState = 0;
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW ) {
    regEnable = !regEnable;
    Serial.println("reg: "  + String(regEnable) );
  }
  gpsdata();
  smartdelay(1000);
}
