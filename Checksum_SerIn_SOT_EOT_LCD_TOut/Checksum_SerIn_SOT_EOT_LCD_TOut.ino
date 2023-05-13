// Created by Upali Pathirana
// https://forum.arduino.cc/t/calculating-check-sum/45883 
// Method used in above: Checksum = (0x100 - sum of bytes)
// abov result Checksum = 2's complement of sum of bytes
// implemented here as Checksum = ((sum of data bytes) XOR 0xFF ) + 1)
// Output results to LCD1602 via I2C Interface -  
// Library version:1.1
#include <LiquidCrystal_I2C.h> // 
//LiquidCrystal_I2C lcd(0x27,20,4);
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//========================================================
const int ledPin =  LED_BUILTIN;// the number of the LED pin
int ledState = LOW;
//========================================================

bool timeOut = false;
bool clearLcd = false;
unsigned long currentMillis ;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 3000;           // interval timeout )

//========================================================
char dataString[16] = {0};
String lcdStr;// String for LCD
byte sum = 0;
byte numBytes = 32;
byte calculated_cksum = 0; // Made global in V2A 


//Unomment following two lines when real Serial
//byte receivedBytes[numBytes];   // an array to store the received data
//bool newData = false;

//Comment out following two lines when real Serial
//bool newData = true; // flag rcvd byte so once when started (Debug)
bool newData = false; // Will be set when Data received. See above for Debug
byte receivedBytes[8] = {0x02, 0x20, 0x00, 0x00, 0x1D, 0x80, 0x42, 0x03}; // Sample Array of Bytes of received simulated
//byte buf[8]; // Buffer array received msg
//========================================================
bool stxOk = false;
bool etxOk = false;
char startMarker = 0x02;// STX
char endMarker = 0x03;// ETX
byte numReceived = 0; // number of bytes received from ETX to STX
//========================================================
void setup()
{
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  delay(100);

  // Remove Serial prints after debug

  Serial.println("V2A Checksum Test Serial Simulation");//
  /*
    Serial.println("Please enter each Nibble in keyboard. Max 16");//
    Serial.println("Confirm with send");//
  */
  lcd.init();                      // initialize the lcd

  lcd.backlight(); // backlight ON
  delay(500);
  lcd.noBacklight(); // backlight OFF
  delay(1000);
  lcd.backlight(); // backlight ON
  delay(1000);
  // Print a message to the LCD.
  lcsWaiting();
  delay(1000);
}
//========================================================
//********************************************************
void loop()
{
  recvBytesWithStartEndMarkers(); // Get serial Data
  if (newData == true)
  {
    showNewData();
    displayLcdLine1(); // Display "Data Received" Text
    lcd.setCursor(0, 1); // 2nd line
    displayRcvdBytesLcdV2();
    // Remove Serial after DEBUG
    displayRcvdBytes(); // Serial
    calcSum(); // Calculate sum
    calculated_cksum = (sum ^ 0xFF)+1; // This is as same as calculated_cksum = -sum used in V2
    displayData();// Send 
    newData = false;
  }
  // Display done

  //timer without using delay
  currentMillis = millis(); // check millis
  if (timeOut == true)
  {
    if (currentMillis - previousMillis >= interval)
    {

      // comment out following Serial Output after testing DEBUG

      //Serial.print("Sum of received data bytes                       = ");
      //printHexByte(sum);

      // move data to first line ans show SUM & Calculated CheckSum in second line
      // show received data in first line
      lcd.clear();        // clear display
      lcd.setCursor(0, 0);
      displayRcvdBytesLcdV2();
      lcd.setCursor(0, 1); // Position Column 0 , Line 1
      lcd.print("S:");
      printHexByteLcd(sum); // display sum of bytes received = XOR
      lcd.print(" C:");
     // byte calculated_cksum = -sum; // made global in V2A
      printHexByteLcd(calculated_cksum); // display calculated Checksum
      lcd.print(" R:");
      byte crcbyte = receivedBytes[numReceived - 2]; // display received Checksum
      printHexByteLcd(crcbyte);

      byte errorFlag = sum + crcbyte;
      if (errorFlag == 0)
      {
        lcd.print("OK");
      }
      else
      {
        lcd.print("XX");
        delay(500);
        lcd.noBacklight(); // backlight OFF
        delay(500);
        lcd.backlight(); // backlight ON
        delay(500);
        lcd.noBacklight(); // backlight OFF
        delay(500);
        lcd.backlight(); // backlight ON
      }

      // Reset Timer Falg & LED
      previousMillis = currentMillis;
      timeOut = false;
      digitalWrite(ledPin, LOW);
    }// close interval exceeded
  }// close if timeout

  delay(1000);
} // End of Loop

//********************************************************
//Fn #01
void lcsWaiting()
{
  lcd.clear();        // clear display
  lcd.setCursor(0, 0); // Position Column 0 , Line 0
  lcd.print("DATA Monitor V2A");
  lcd.setCursor(0, 1); // Position Column 0 , Line
  lcd.print("Waiting....");//
}
//----------------------------------------------------
//Fn#02
void recvWithEndMarker()
{
  static byte ndx = 0;
  char endMarker = 0x03;// STX
  char rc;

  while (Serial.available() > 0 && newData == false)
  {
    rc = Serial.read();

    if (rc != endMarker)
    {
      receivedBytes[ndx] = rc;
      ndx++;
      /*if (ndx >= numBytes) //changed from >=
           // above removed. String is not limited
            //{
              Serial.print("Buffer exceeded "); // inserted by U.P.
              ndx = numBytes - 1;
            }
      */
    }
    else
    {

      receivedBytes[ndx + 1] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

//---------------------------------------------------
//Fn#02A
void recvBytesWithStartEndMarkers()
{
  // STX = 0x02 anfd ETX = 0x03 alredy set in Global variables
  static boolean recvInProgress = false; // Note Ststic. will be initialized once only
  static byte ndx = 0;
  byte receivedByte;
  while (Serial.available() > 0 && newData == false)
  {
    receivedByte = Serial.read();
    receivedBytes[ndx] = receivedByte; // moved from beloe to include STX
    ndx++;
    if (recvInProgress == true  && receivedByte == startMarker)// receiving STX during rcv n progress
    {
      ndx = 1;
    }
    if (recvInProgress == true) {
      if (receivedByte != endMarker)
      {
        // receivedBytes[ndx] = receivedByte;
        //  ndx++;
        if (ndx >= numBytes)
        {
          Serial.println("Buffer exceeded!!! ");
          ndx = numBytes - 1;
        }
      }
      else
      {
        receivedBytes[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        numReceived = ndx;  // save the number for use when printing
        ndx = 0;
        newData = true;
      }
    }

    else if (receivedByte == startMarker)
    {
      recvInProgress = true;
    }
  }
}

//----------------------------------------------------
//Fn#03
void displayLcdLine1()
{
  lcd.clear();        // clear display
  lcdStr = ("Data received");
  lcd.setCursor(0, 0);
  lcd.print(lcdStr);
}
//----------------------------------------------------
//Fn#04A
void displayRcvdBytesLcdV1()
{
  for (int i = 0; i < numBytes; i++)
  {
    printHexByteLcd(receivedBytes[i]);
  }
}
//---------------------------------------------------------
//Fn#04B
void printHexByteLcd(byte x)
{
  if (x < 16)
  {
    lcd.print('0');
  }
  lcd.print(x, HEX);
}
//==========================================================
//Fn#05
void displayRcvdBytesLcdV2()
{
  sprintf(dataString, "%02X%02X%02X%02X%02X%02X%02X%02X",
          receivedBytes[0], receivedBytes[1], receivedBytes[2], receivedBytes[3],
          receivedBytes[4], receivedBytes[5], receivedBytes[6], receivedBytes[7]);
  lcd.print(dataString);
  previousMillis = millis(); // set previous to current millis
  timeOut = true; //flag to start timer
  digitalWrite(ledPin, HIGH);
}
//---------------------------------------------------------

void calcSum()
{
  sum = 0x00; //removed in V2A from 0xFF
  // Calculate the sum excluding the first STX (0x02) byte.
  // The last two bytes are the received checksum and ETX (0x03).
  // Both are not used in the sum calculation.
  for (int i = 1; i < numReceived - 2; i++)
  {
    sum += receivedBytes[i]; // add all bytes in Array
  }
}
//---------------------------------------------------------

void displayRcvdBytes()
{
  Serial.print("Received data bytes: ");
  for (int i = 0; i < numReceived; i++)
  {
    printHexByte(receivedBytes[i]);
    Serial.print("  ");
  }
  Serial.println();
}
//---------------------------------------------------------
void displayData()
{
  Serial.print("Sum of received data bytes                       = ");
  printHexByte(sum);
  Serial.println();

  //byte calculated_cksum = -sum; // changed V2A now in loop for clarity
  Serial.print("Calculated checksum                              = ");
  printHexByte(calculated_cksum);
  Serial.println();

  byte crcbyte = receivedBytes[numReceived - 2];
  Serial.print("Received checksum byte                           = ");
  printHexByte(crcbyte);
  Serial.println();

  byte overall = sum + crcbyte;
  Serial.print("Sum of received data bytes and received checksum = ");
  printHexByte(overall);
  Serial.println();
  Serial.println();
}
//---------------------------------------------------------
void printHexByte(byte x)
{
  Serial.print("0x");
  if (x < 16) {
    Serial.print('0');
  }
  Serial.print(x, HEX);
  // Serial.println();
}
//---------------------------------------------------------
//====================================================
void showNewData()
{
  if (newData == true)
  {
    // printHexByte(numReceived);
    Serial.print(numReceived, HEX);
    // Serial.println();
    Serial.println("  HEX values received... ");
    for (byte n = 0; n < numReceived; n++)
    {
      printHexByte(receivedBytes[n]);
      Serial.print(' ');
    }
    Serial.println();
    Serial.println();
    newData = false;
  }
}
//====================================================
