#include <LiquidCrystal.h>
#include <SPI.h>
#include <Adafruit_ATParser.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BLEBattery.h>
#include <Adafruit_BLEEddystone.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BLEMIDI.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined(ARDUINO_ARCH_SAMD)
  #include <SoftwareSerial.h>
#endif
#include <Adafruit_CircuitPlayground.h>
#include <BluefruitConfig.h>
LiquidCrystal lcd(6, 5, 9, 10, 11, 12);
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
// Degree symbol bitmap
byte degree[8] = {
  B01000,
  B10100,
  B01000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};
 Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
 
//TMP36 Pin Variables
const int sensorPin = A0;   //the analog pin the TMP36's Vout (sense) pin is connected to
            
//Set to 1 to display Celsius instead of Fahrenheit
int celsius = 1 ;

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
  }
void setup() {
  //Create the degree symbol bitmap
  lcd.createChar(0, degree);
 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit HID Keyboard Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Bluefruit Keyboard': "));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Bluefruit Keyboard Pasteque" )) ) {
    error(F("Could not set device name?"));
  }

  /* Enable HID Service */
  Serial.println(F("Enable HID Service (including Keyboard): "));
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    if ( !ble.sendCommandCheckOK(F( "AT+BleHIDEn=1" ))) {
      error(F("Could not enable Keyboard"));
    }
  }else
  {
    if (! ble.sendCommandCheckOK(F( "AT+BleKeyboardEn=On"  ))) {
      error(F("Could not enable Keyboard"));
    }
  }

  /* Add or remove service requires a reset */
  Serial.println(F("Performing a SW reset (service changes require a reset): "));
  if (! ble.reset() ) {
    error(F("Couldn't reset??"));
  }

  Serial.println();
  Serial.println(F("Go to your phone's Bluetooth settings to pair your device"));
  Serial.println(F("then open an application that accepts keyboard input"));

  Serial.println();
  Serial.println(F("Enter the character(s) to send:"));
  Serial.println(F("- \\r for Enter"));
  Serial.println(F("- \\n for newline"));
  Serial.println(F("- \\t for tab"));
  Serial.println(F("- \\b for backspace"));

  Serial.println();
}
 
 
void loop() {
 
  //Clear LCD
  lcd.clear();
 
  //Display Currently on the LCD
  lcd.print("Currently ") ;
 
  //getting the voltage reading from the temperature sensor
  int reading = analogRead(sensorPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = reading * 3.3;
  voltage /= 1024.0;
 
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree with 500 mV offset
                          //to degrees ((voltage - 500mV) times 100)
 
  if (celsius == 1) //If you set temperature as Celsius it will print Celsius values
  {
    //Round the temperature to a whole number
    float roundedTempC = round(temperatureC);
    ble.print("AT+BLEKEYBOARD=");
    ble.println(roundedTempC);
    ble.println("OK");
    // Display temperature in C
    if( ble.waitForOK() )
    {
      Serial.println( F("OK!") );
    }else
    {
      Serial.println( F("FAILED!") );
    }
    lcd.print(roundedTempC, 0);
    lcd.write(byte(0)); //Degree symbol we created earlier
    lcd.print("C");
    
  }
  else //Display in Fahrenheit
  {
    //Convert from Celsius to Fahrenheit
    float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
 
    //Round the temperature to a whole number
    float roundedTempF = round(temperatureF);
 
    // Display temperature in F
    lcd.print(roundedTempF, 0);
    lcd.write(byte(0)); //Degree symbol we created earlier
    lcd.print("F");
  }
  delay(2000);
}
