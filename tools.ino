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
  display.print(eCO2);
  if (eCO2 < 1000)
  {
    display.println("ppm");
  } else 
  {
    display.println();
  }

  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.print(F("T:"));
  display.print(temperature);
  display.println(F("C"));

  display.setTextColor(SSD1306_WHITE);
  display.print(F("H:"));
  display.print(humidity);
  display.println(F("%"));

  if (preheatSec > 0) {
    display.setTextSize(2);    
    display.print("PREHEAT");
    display.print(preheatSec);  
    preheatSec--;
  }
  else 
  {      
    display.setTextSize(2);    
    display.print("CO2:");
    display.print(ppm);
    if (ppm < 1000)
    {
      display.print("ppm");
    }
  }

  display.display();
}

void PWM_ISR() {
  long tt = millis();
  int val = digitalRead(pwmPin);
  
  if (val == HIGH) {    
    if (val != prevVal) {
      h = tt;
      tl = h - l;
      prevVal = val;
    }
  }  else {    
    if (val != prevVal) {
      l = tt;
      th = l - h;
      prevVal = val;
      ppm = 2000 * (th - 2) / (th + tl - 4);      
    }
  }
}

void displayPreheating(int secLeft) {
  display.setTextSize(2);    
  display.print("PREHEAT");
  display.print(secLeft);  
  display.display();   
}

void displayPPM(long ppm) {
  display.setTextSize(2);    
  display.println("CO2    PPM");    
  display.setTextSize(1);    
  display.println();    
  display.setTextSize(5);    
  if (ppm < 1000) {
    display.print(" ");
  }
  display.print(ppm);  
  display.display();
  Serial.println(ppm);  
}

void blinkRgbLed(RGB col){
  for(int i=0;i<=3; i++)
  {
    WiFiDrv::analogWrite(RED, col.red);   //RED
    WiFiDrv::analogWrite(GREEN, col.green); //GREEN
    WiFiDrv::analogWrite(BLUE, col.blue);   //BLUE
    delay(100);
    WiFiDrv::analogWrite(RED, 0);   //RED
    WiFiDrv::analogWrite(GREEN, 0); //GREEN
    WiFiDrv::analogWrite(BLUE, 0);   //BLUE
    delay(100);
  }
}
