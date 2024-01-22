void setupBME()
{
  unsigned status;

  status = bme.begin(0x76, &Wire);
  // Initialize BME280 sensor
  if (!status) 
  {
        debugln("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        debug("SensorID was: "); debugln(bme.sensorID());
        debug("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        debug("   ID of 0x56-0x58 represents a BMP 280,\n");
        debug("        ID of 0x60 represents a BME 280.\n");
        debug("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
  }
}

void drawDisplay(void) {
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  display.print(F("CO2:"));
  display.print(ppm);
  if (ppm < 1000)
  {
    display.println("ppm");
  } else 
  {
    display.println();
  }

  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.print(F("T:"));
  display.print(temperature);
  display.println(F(" C"));

  display.setTextColor(SSD1306_WHITE);
  display.print(F("H:"));
  display.print(humidity);
  display.println(F(" %"));

  display.setTextColor(SSD1306_WHITE);
  display.print(F("P:"));
  display.print((int)pressure/100);
  display.println(F(" Mb"));

  display.display();
}

void blinkRgbLed(RGB col){
  WiFiDrv::analogWrite(RED, col.red);   //RED
  WiFiDrv::analogWrite(GREEN, col.green); //GREEN
  WiFiDrv::analogWrite(BLUE, col.blue);   //BLUE
  delay(1500);
  WiFiDrv::analogWrite(RED, 0);   //RED
  WiFiDrv::analogWrite(GREEN, 0); //GREEN
  WiFiDrv::analogWrite(BLUE, 0);   //BLUE
}

// Attach the interrupt handler to the SERCOM
void SERCOM3_Handler()
{
  S8_serial.IrqHandler();
}