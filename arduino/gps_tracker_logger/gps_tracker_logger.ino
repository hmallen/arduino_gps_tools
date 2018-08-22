//#include <SPI.h>
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include <TinyGPS++.h>

// GPS pins
#define rxPin 7
#define txPin 255
// UTC timezone offset
#define TZ_OFFSET -5
// Header to identify individual elements of logged data
#define CSV_HEADER "Date/Time,Satellites,HDOP,Latitude,Longitude,Age,Altitude,Course,Speed"

// Chip Select
const int chipSelect = SS;

const int logInterval = 5000;
unsigned long dataLast = 0;

SdFat sd;
SoftwareSerial gpsSerial(rxPin, txPin);
TinyGPSPlus gps;

File logFile;

void dateTime(uint16_t* date, uint16_t* time) {
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(), month(), day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(), minute(), second());
}

void setup() {
  Serial.begin(115200);

  gpsSerial.begin(9600);

  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  Serial.print("Waiting for valid GPS data to set date/time...");
  while (true) {
    if (gpsSerial.available()) {
      while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
      }
    }
    if (gps.date.isUpdated() && gps.time.isUpdated()) {
      if (gps.date.isValid() && gps.time.isValid()) {
        if (gps.date.age() < 500 && gps.time.age() < 500) {
          // Set date/time with current values from GPS
          setTime(
            gps.time.hour(), gps.time.minute(), gps.time.second(),
            gps.date.day(), gps.date.month() , gps.date.year()
          );
          adjustTime(TZ_OFFSET * SECS_PER_HOUR);
          break;
        }
      }
    }
  }
  Serial.println("complete.");

  /*
    char filename[15];
    strcpy(filename, "GPSLOG00.TXT");
    for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i / 10;
    filename[7] = '0' + i % 10;
    // create if does not exist, do not open existing, write, sync after write
    if (!sd.exists(filename)) {
      break;
    }
    }
  */

  // Create log file name based on current datetime
  String filenameString = "GPSLog_";
  filenameString += formatDigit(month()) + formatDigit(day()) + String(year()) + "-";
  filenameString += formatDigit(hour()) + formatDigit(minute()) + formatDigit(second());
  filenameString += ".csv";

  char filename[filenameString.length() + 1];
  filenameString.toCharArray(filename, (filenameString.length() + 1));

  SdFile::dateTimeCallback(dateTime);

  if (!logFile.open(filename, O_CREAT | O_WRITE | O_EXCL)) {
    sd.errorHalt();
  }

  Serial.print("File Name: "); Serial.println(filename);

  // Write data header to top of log file
  logData(CSV_HEADER);

  Serial.println("Setup complete.");
}

void loop() {
  // Update GPS data when available
  if (gpsSerial.available()) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
  }

  // Log data at interval
  if ((millis() - dataLast) > logInterval) {
    //digitalClockDisplay();
    logData(logString());

    dataLast = millis();
  }
}

void logData(String dataString) {
  if (logFile) {
    logFile.println(dataString);
    logFile.flush();

    Serial.print("Logged data: "); Serial.println(dataString);
  }
  else {
    Serial.println("Log file not available.");
  }
}

String logString() {
  String logString = "";

  // ISO 8601 formatted UTC datetime
  logString += String(gps.date.year()) + "-";
  logString += formatDigit(gps.date.month()) + "-";
  logString += formatDigit(gps.date.day()) + "T";
  logString += formatDigit(gps.time.hour()) + ":";
  logString += formatDigit(gps.time.minute()) + ":";
  logString += formatDigit(gps.time.second()) + "Z,";

  /*
    logString += formatDigit(gps.date.month()) + "/";
    logString += formatDigit(gps.date.day()) + "/";
    logString += formatDigit(gps.date.year()) + ",";
    int hourDelta = gps.time.hour() + TZ_OFFSET;
    int hourLocal;
    if (hourDelta < 0) hourLocal = 24 + hourDelta;
    else if (hourDelta > 23) hourLocal = -1 + hourDelta;
    logString += formatDigit(hourLocal) + ":";
    logString += formatDigit(gps.time.hour()) + ":";
    logString += formatDigit(gps.time.minute()) + ":";
    logString += formatDigit(gps.time.second()) + ",";
  */

  // Location & Precision Info
  logString += String(gps.satellites.value()) + ",";
  logString += String(gps.hdop.hdop()) + ",";
  logString += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + ",";
  logString += String(gps.location.age()) + ",";
  logString += String(gps.altitude.feet()) + ",";
  logString += String(gps.course.deg()) + ",";
  logString += String(gps.speed.mph());

  return logString;
}

String formatDigit(uint32_t digit) {
  String digitString = "";
  if (digit < 10 || digit == 0) {
    digitString += "0";
  }
  digitString += String(digit);
  return digitString;
}

/*
  // Time Library Functions
  void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year());
  Serial.println();
  }

  void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
  }
*/
