
#include <OneWire.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include "FreeSerifBoldItalic18pt7b.h"
#include <Adafruit_SH110X.h>
#include <TinyGPSPlus.h>
#include <math.h>


#include <SPI.h>
#include <SD.h>

TinyGPSPlus gps;
int ACCURACY = 15;
int ALTACCURACY = 20;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ONE_WIRE_BUS D3
#define CS_PIN  D8

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
File fi;
bool regEnable = false;
bool endRequest = false;
bool startRequest = false;
bool displayOn = true;
unsigned int displayTimeOut = 120;
int fileNum = 0;
bool sdOn = false;
//course
double lt = 0;
double lg = 0;
double dist = 0;
double altPos = 0;
double altNeg = 0;
double altStart = 0;
int bat_percentage = 0;

char temperatureString[6];
const int led = 13;
String fileName = "";
int timeZone = -3;
bool initilalized = false;
int buttonPin = D4;

uint32_t timerData;
uint32_t timerReg;
uint32_t timerOled;
uint32_t timerBat = 99999999;
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
void getLastFile() {
  File dir = SD.open("/regs" );
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      break;
    }
    String ename = String(entry.name());
    char fname[ename.length() + 1] ;
    ename.toCharArray(fname, (ename.length() + 1));
    int num = 0;
    if (ename.length() > 18) {
      num = (String(strtok(fname, "-")[13]) +  String(strtok(fname, "-")[14])).toInt();
    } else {
      num = String(strtok(fname, "-")[13] ).toInt();
    }
    if (fileNum < num) {
      fileNum = num;
      fileName = ename;
    }
  }
}
String numToTwoDigits(int num )
{
  return ((num < 10) ? "0" : "") +  String(num);
}
void registerData()
{
  if (regEnable) {
    File fiReg;

    //int currentMonth = gps.date.month();
    //int currentDay = gps.date.day();
    int currentYear = gps.date.year();
    if (fileName == "") {
      fileNum = fileNum + 1;
      fileName =  "route_" + numToTwoDigits(localizeHourTime(gps.time.hour())) + numToTwoDigits(gps.time.minute())  + numToTwoDigits(gps.time.second()) + "-" + String(fileNum) + ".gpx";
    }
    if (currentYear > 2000) {
      if (!SD.exists("/regs"))
      {
        SD.mkdir("/regs");
      }
      String datetime = String(gps.date.year())  + "-" +  numToTwoDigits(gps.date.month()) + "-" + numToTwoDigits(gps.date.day()) + "T" + numToTwoDigits(localizeHourTime(gps.time.hour())) + ":" + numToTwoDigits(gps.time.minute()) + ":" + numToTwoDigits(gps.time.second()) + "Z" ;
      if (!SD.exists("/regs/" + fileName))
      {
        fiReg = SD.open("/regs/" + fileName, FILE_WRITE);
        if (fiReg) {
          if (gps.altitude.isValid()) {
            fiReg.println(  "type,latitude,longitude,altitude (m),time,name,desc,speed (km/h)" );
            fiReg.println( "T," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," + String(gps.altitude.meters())  + "," + datetime + ",start," + String(currentTemp) + "," + String(gps.speed.kmph()) );
            fiReg.close();
          }
        }
      } else {
        if (gps.altitude.isValid()) {
          fiReg = SD.open("/regs/" + fileName, FILE_WRITE);
          if (fiReg) {
            fiReg.println( "T," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," + String(gps.altitude.meters()) + "," + datetime + ",," + String(currentTemp) + "," + String(gps.speed.kmph()));
            fiReg.close();
          }
        }
      }
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
  display.setTextColor(SH110X_WHITE);
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
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
#define R 6371000.0
#define PI 3.1415926535897932384626433832795
  double phi1 = lat1 * PI / 180.0;
  double phi2 = lat2 * PI / 180.0;
  double dphi = (lat2 - lat1) * PI / 180.0;
  double dlambda = (lon2 - lon1) * PI / 180.0;
  double a1 = sin(dphi / 2.0) * sin(dphi / 2.0);
  double a2 = cos(phi1) * cos(phi2);
  double a3 = sin(dlambda / 2.0) * sin(dlambda / 2.0);
  double a = a1 + (a2 * a3);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  double d = R * c;
  return d;
}
bool setConfigs(String key, String val)
{

  String filecont = "";
  Serial.println("open file");
  fi = SD.open("/configs/configs.ini");
  String v = "";
  if (fi) {
    while (fi.available()) {
      v = fi.readStringUntil(',');
      filecont += v ;
      filecont += ",";
      if (v == key)
      {
        Serial.println("key found");
        Serial.println("value: " + val);
        filecont += val;
        filecont += "\r\n";
        fi.readStringUntil('\r');
        fi.readStringUntil('\n');
        Serial.println("val put");
      } else {
        v = fi.readStringUntil('\r');
        fi.readStringUntil('\n');
        filecont += v;
        filecont += "\r\n";
      }
    }
    fi.close();
    Serial.println("file closed");
  }
  return true;
}
String getConfigs(String key)
{
  String k = "";
  String v = "";
  fi = SD.open("/configs/configs.ini");
  if (fi) {
    while (fi.available()) {
      k = fi.readStringUntil(',');
      v = fi.readStringUntil('\r');
      fi.readStringUntil('\n');
      if (k == key)
      {
        fi.close();
        //if (v == "")return NULL;
        return v;
      }
    }
  }
  fi.close();
  return "";

}
void gpsdata()
{
  Serial.println("gps print");

  if (!initilalized) {
    initializingGPS();
  }

  currentTemp = (int)trunc(round(getTemperature()));
  if (false) {
    Serial.println("DONE-----------------------------------");
    Serial.println("gps data");
    Serial.println( numToTwoDigits(gps.date.day()) + "/" + numToTwoDigits(gps.date.month()) + "/" + String(gps.date.year()) + " - " + numToTwoDigits(localizeHourTime(gps.time.hour())) + ":" + numToTwoDigits(gps.time.minute()) + ":" + numToTwoDigits(gps.time.second())  );
    Serial.println("lat: " + String(gps.location.lat(), 6) + " long: " + String(gps.location.lng(), 6) );
    Serial.println("vel: "  + String(gps.speed.kmph()) + "Km/h");
    Serial.println("alt: "  + String(gps.altitude.meters()) + "mts");
    Serial.println("sat's: "  + String(gps.satellites.value()));
    Serial.println("pres: "  + String(gps.hdop.value()) + "hdop");
    Serial.println("temp: "  + String(currentTemp) );
    Serial.println("time to get data: "  + String((millis() - timerData) / 1000));
  }


  if (gps.altitude.isValid() )
  {
    if (displayOn) {

      initilalized = true;
      display.clearDisplay();
      //Battery display and check
      if (millis() - timerBat > (60 * 1000)) {
        batCheck();
        timerBat = millis();
      }
      display.drawRect(107, 0, 20, 8, 1);
      display.fillRect(127, 3, 2, 2, 1);
      display.fillRect(109, 2, map(bat_percentage, 1, 100, 0, 16), 4, 1);


      display.setTextSize(1);

      display.setTextColor(SH110X_WHITE);
      display.setCursor(1, 1);
      display.print( numToTwoDigits(gps.date.day()) + "/" +  numToTwoDigits(gps.date.month()) + "/" + String(gps.date.year() - 2000) + "-" + numToTwoDigits(localizeHourTime(gps.time.hour()) ) + ":" + numToTwoDigits(gps.time.minute()) + ":" + numToTwoDigits(gps.time.second()));

      display.setCursor(1, 10);
      display.print("alt: "  + String(gps.altitude.meters()) + "mts");

      display.setCursor(89, 10);
      display.print("t:" + String(currentTemp) + "c");

      display.setCursor(1, 20);
      display.print("sat's:"  + String(gps.satellites.value()));

      display.setCursor(55, 20);
      display.print("v:"  + String(gps.speed.kmph()) + "Kmh");

      display.setCursor(1, 30);
      if (!regEnable) {
        display.print(String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6));
      }
      else {
        display.print("trip: " + ((dist < 1000) ? String(dist) + "mts" : String(dist / 1000) + "kms") );
        display.setCursor(1, 40);
        display.print("pos: " + String((int)round(altPos)) + "m neg: " +   String((int)round(altNeg)) + "m" );
      }
      display.display();

      if (startRequest) {
        regEnable = true;
        startRequest = false;
        //reset trip
        dist = 0;
        lg = 0;
        lt = 0;
        altPos = 0;
        altNeg = 0;
        altStart = gps.altitude.meters();
        //--------------
        display.setCursor(1, 48);
        int buttonState = 0;
        buttonState = digitalRead(buttonPin);
        if ( fileName != "" && buttonState == LOW) {
          Serial.println("endRequest: "  + String(endRequest) );
          display.println("Keep pressing to      create new file");
          display.display();
          smartdelay(1000);
          buttonState = digitalRead(buttonPin);
          if (buttonState == LOW ) {
            fileName = "";
            display.fillRect(0, 48, 128, 20, 0);
            display.display();
            display.setCursor(1, 50);
            display.print("file: route_" + String(gps.time.hour() + timeZone) +  String(gps.time.minute())  + String(gps.time.second()) + ".gpx" ) ;
            display.display();
            smartdelay(1000);
          }

          display.fillRect(0, 48, 128, 20, 0);
        }
      } else {
        if (endRequest) {
          regEnable = false;
          endRequest = false;
          display.fillRect(0, 48, 128, 20, 0);
          display.setCursor(1, 50);
          display.print("End recording");
          display.display();
          endRequest = false;
          smartdelay(1000);
        }
      }
      if (regEnable) {
        display.setCursor(1, 50);
        display.print("Recording");
        bool flash = true;
        for (int i = 0; i < 4; i++) {
          display.fillCircle(62, 54, 4, flash ? 1 : 0);
          flash = !flash;
          display.display();
          smartdelay(500);
        }
      } else {
        if (!sdOn) {
          display.setCursor(1, 50);
          display.print("No SD");
          display.display();
        }
      }
    } else {
      display.clearDisplay();
      display.display();
    }
    if (regEnable) {

      altPos = (gps.altitude.meters() - (altStart  + altPos) > ALTACCURACY) ? (gps.altitude.meters() - altStart) : altPos;
      altNeg = (gps.altitude.meters() - (altStart + altNeg)  < -(ALTACCURACY)) ? (gps.altitude.meters() - altStart) : altNeg;

      double pdist = 0;
      if (lg != 0) {
        pdist = calculateDistance(lt, lg , gps.location.lat(), gps.location.lng());
        if (pdist > ACCURACY) {
          dist += pdist;
          lg = gps.location.lng();
          lt = gps.location.lat();
          registerData();
        }
        Serial.println(String(dist));
      } else {
        lg = gps.location.lng();
        lt = gps.location.lat();
        registerData();
      }
    }
  }
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void batCheck()
{
  float voltage;
  int analogInPin  = A0;    // Analog input pin
  int sensorValue;
  float calibration = -0.20; // Check Battery voltage using multimeter & add/subtract the value
  sensorValue = analogRead(analogInPin);
  // (31k/(100k+31k)) * 4.2V = sensor V.
  //voltage = (((sensorValue * 3.3) / 1024) * 2 + calibration); //multiply by two as voltage divider network is 100K & 100K Resistor
  //bat_percentage = mapfloat(voltage, 2.8, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
  voltage = (sensorValue / 0.230769230769) / 1000; //multiply by two as voltage divider network is 100K & 100K Resistor
  voltage = voltage + calibration;
  bat_percentage = mapfloat(voltage, 3, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
  bat_percentage = (bat_percentage >= 100) ? 100 : ((bat_percentage <= 0) ? 1 : bat_percentage);
  if (!initilalized) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(1, 1);
    display.print("sensorValue: "  + String(sensorValue));
    display.setCursor(1, 10);
    display.print("voltage real: "  + String(voltage - calibration));
    display.setCursor(1, 30);
    display.print("voltage: "  + String(voltage));
    display.setCursor(1, 40);
    display.print("bat_percentage: "  + String(bat_percentage));
    display.display();
    while (digitalRead(buttonPin) == HIGH) {
      delay(80);
    }
  }

}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    //Serial.println(String(Serial.available()));
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}

void setup(void) {

  Serial.begin(9600);
  Serial.println(F("init ok"));
  //if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  // set the data rate for the SoftwareSerial port
  Serial.println("Starting...");

  pinMode(buttonPin, INPUT);

  if (!SD.begin(CS_PIN)) {
    Serial.println("initialization failed!");
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(30, 15);
    display.println("No SD");
    display.setTextSize(1);
    display.setCursor(10, 35);
    display.println("tap to continue..");
    display.display();
    sdOn = false;
    while (digitalRead(buttonPin) == HIGH) {
      delay(80);
    }

  } else {
    sdOn = true;
    initializingGPS();
    //If there is SD memory on, get configs
    String val = "";
    val = getConfigs("accuracy");
    Serial.println("accuracy: " + val);
    ACCURACY  = val != "" ? val.toInt() : ACCURACY;
    val = getConfigs("displayTimeOut");
    Serial.println("displayTimeOut: " + val);
    displayTimeOut = val != "" ? val.toInt() : displayTimeOut;
    val = getConfigs("altAccuracy");
    Serial.println("ALTACCURACY: " + val);
    ALTACCURACY = val != "" ? val.toInt() : ALTACCURACY;
  }
  getLastFile();
  timerOled = millis();
  timerData = millis();
  initializingGPS();
  delay(1000);
  batCheck();
}
void loop(void) {
  //reads for 1 seg gps data
  smartdelay(1000);
  // check if the pushbutton is pressed.
  if ((millis() - timerOled > displayTimeOut * 1000) && initilalized) {
    //apagar display
    Serial.println("Display OFF");
    displayOn = false;
  }
  int buttonState = 0;
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW )
  {
    timerOled = millis();
    if (!displayOn) {
      Serial.println("Display ON");
      displayOn = true;
    } else {
      if (sdOn) {
        endRequest = regEnable ? true : false;
        startRequest = regEnable ? false : true;
      }
    }
  }
  gpsdata();

}
