#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RED     25
#define GREEN   26
#define BLUE    27

// DEBUG
#define DEBUG3
#define DEBUG2
#define DEBUG1 

#ifdef DEBUG2 
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif


#define pwmPin 7
int preheatSec = 120;
int prevVal = LOW;
long th, tl, h, l, ppm = 0;


Adafruit_SGP30 sgp;
Adafruit_BME280 bme; // I2C

float temperature;
float humidity;
float pressure;
uint16_t eCO2;
uint16_t TVOC;


/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(115200);
  //while (!Serial) { delay(10); } // Wait for serial console to open!
  
  setupBME();
  
  WiFiDrv::pinMode(RED, OUTPUT);
  WiFiDrv::pinMode(GREEN, OUTPUT);
  WiFiDrv::pinMode(BLUE, OUTPUT);

  Serial.println("SGP30 test");

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }

  pinMode(pwmPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pwmPin), PWM_ISR, CHANGE);  

  // Clear the buffer
  display.clearDisplay();

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}

int counter = 0;
void loop() {
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  //pressure = bme.readPressure();

  //float temperature = 22.1; // [°C]
  //float humidity = 45.2; // [%RH]
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }

  eCO2 = sgp.eCO2;
  TVOC = sgp.TVOC;
  Serial.print("TVOC "); Serial.print( TVOC ) ; Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(eCO2); Serial.println(" ppm");

  if( eCO2 >= 400 && eCO2 <= 1000 )
  {
    WiFiDrv::analogWrite(RED, 0);   //RED
    WiFiDrv::analogWrite(GREEN, 32); //GREEN
    WiFiDrv::analogWrite(BLUE, 0);   //BLUE
  }
  else if( eCO2 > 1000 && eCO2 <= 2000 )
  {
    WiFiDrv::analogWrite(RED, 32);   //RED
    WiFiDrv::analogWrite(GREEN, 32); //GREEN
    WiFiDrv::analogWrite(BLUE, 0);   //BLUE
  } else if( eCO2 > 2000 && eCO2 <= 5000 )
  {
    WiFiDrv::analogWrite(RED, 32);   //RED
    WiFiDrv::analogWrite(GREEN, 0); //GREEN
    WiFiDrv::analogWrite(BLUE, 0);   //BLUE
  }

  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
  Serial.print("Temp.: "); Serial.print(temperature); Serial.print("\t");
  Serial.print("Hum.: "); Serial.print(humidity); Serial.println("");
  // Serial.print("Pres.: "); Serial.println(pressure / 100.0F);

  counter++;
  if (counter == 60) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }

  drawDisplay();
  
  delay(1000);
 }
