#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include "wiring_private.h"
#include "s8_uart.h"

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
//#define DEBUG2
#define DEBUG1 

#ifdef DEBUG2 
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif


//#define pwmPin 7
int preheatSec = 120;
int prevVal = LOW;
long th, tl, h, l, ppm = 0;

struct RGB {
  byte red;
  byte green;
  byte blue;
};

RGB colors = {0, 32, 0};

//Adafruit_SGP30 sgp;
Adafruit_BME280 bme; // I2C

float temperature;
float humidity;
float pressure;

// Add a new Serial interface
Uart S8_serial (&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0); // Create the new UART instance assigning it to pin 1 and 0

S8_UART *sensor_S8;
S8_sensor sensor;

void setup() {
  Serial.begin(115200);
  
  S8_serial.begin(9600);
  sensor_S8 = new S8_UART(S8_serial);

  // Additional Serial interface  
  pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
  pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0

  setupBME();
  
  WiFiDrv::pinMode(RED, OUTPUT);
  WiFiDrv::pinMode(GREEN, OUTPUT);
  WiFiDrv::pinMode(BLUE, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  // Check if S8 is available
  sensor_S8->get_firmware_version(sensor.firm_version);
  int len = strlen(sensor.firm_version);
  if (len == 0) {
      Serial.println("SenseAir S8 CO2 sensor not found!");
      while (1) { delay(1); };
  }

  #ifdef DEBUG1 
    // Show S8 sensor info
    Serial.println(">>> SenseAir S8 NDIR CO2 sensor <<<");

    Serial.print("Firmware version: "); Serial.println(sensor.firm_version);

    sensor.sensor_type_id = sensor_S8->get_sensor_type_ID();
    Serial.print("Sensor type: 0x"); printIntToHex(sensor.sensor_type_id, 3); Serial.println("");

    sensor.sensor_id = sensor_S8->get_sensor_ID();
    Serial.print("Sensor ID: 0x"); printIntToHex(sensor.sensor_id, 4); Serial.println("");

    sensor.map_version = sensor_S8->get_memory_map_version();
    Serial.print("Memory map version: "); Serial.println(sensor.map_version);
  
    sensor.abc_period = sensor_S8->get_ABC_period();
    
    if (sensor.abc_period > 0) {
      Serial.print("ABC (automatic background calibration) period: ");
      Serial.print(sensor.abc_period); Serial.println(" hours");
    } else {
      Serial.println("ABC (automatic calibration) is disabled");
    }  
  #endif

}

void loop() 
{
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure();

  sensor.co2 = sensor_S8->get_co2();
  ppm = sensor.co2;

  if( ppm > 800 && ppm <= 1500 )
    blinkRgbLed({32, 32, 0}); // {32, 32, 0}
  else if( ppm > 1800 )
    blinkRgbLed({32, 0, 0}); // {32, 0, 0}
  else if( ppm < 800)
    blinkRgbLed({0, 0, 0}); // {0, 0, 0}


  #ifdef DEBUG1
    Serial.print("CO2 "); Serial.print(ppm); Serial.print(" ppm"); Serial.print("\t");
    Serial.print("Temp.: "); Serial.print(temperature); Serial.print("\t");
    Serial.print("Hum.: "); Serial.print(humidity); Serial.print("\t");
    Serial.print("Pre.: "); Serial.print(pressure/100); Serial.println("");
  #endif

  drawDisplay();
  
  delay(10000);
}
