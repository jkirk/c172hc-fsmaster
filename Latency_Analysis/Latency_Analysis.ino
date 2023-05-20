// Created by Upali Pathirana
// 2023.05.17
// Test reliability of data transfer @9600 Baud
// Check blocks received within time frame specified in waitRestart
// Send 0x55 to Host to notify Ready to receive data
// calculates Checksum according to 2's complement of sum of bytes (excl. SOT and EOT)
// implemented here as Checksum = ((sum of data bytes) XOR 0xFF ) + 1)
// Count number of blocks received.
// at the end of time frame, display number of blocks received with correct and wrong Checksum caused by latency

#include <LiquidCrystal_I2C.h> // Library version:1.1.2
//LiquidCrystal_I2C lcd(0x27,20,4);
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display


//========================================================
// Update following after changes
String mainPurpose = "Latency Analysis ";
String currentVersion = " V1A";
String fileId = "File-ID: 14D"; // for LCD display
unsigned long transferRate = 9600; // Baud Rate

//========================================================

const int ledPin =  LED_BUILTIN;// the number of the LED pin
int ledState = LOW;
//========================================================
bool clearLcd = false;
unsigned long currentMillis ;
unsigned long lastLcdMillis = 0;        // will store last time LCD was updated

const long waitRestart = 10000; // Timeout for data entry completion
const long pauseDelay = 2000; //waiting time to allow reading results of analysis
unsigned long lastRstMillis = 0; //
bool timeOutLcd = false;
bool timeOutSerial = false;
bool timeOutRestart = false;
unsigned long lastSerialMillis = 0; // will store last time Analysisn Serial out was executed

//========================================================
char dataString[16] = {0};
String lcdStr;// String for LCD
byte sum = 0;
byte numBytes = 32;
byte calculated_cksum = 0; // Made global in V2A
byte rcvd_cksum = 0; // Made global in V2C (formerly crcbyte) now global
byte sumPlusChksum = 0;
byte errorFlag = 0; // Sum of received Bytes + Received Checksum
int dataCounter = 0; //
int errCounter = 0; //
int startCode = 0x55; // Code to send to Host. Timer started to send data

bool newData = false; // Will be set when Data received. See above for Debug
byte receivedBytes[8] = {0x02, 0x20, 0x00, 0x00, 0x1D, 0x80, 0x42, 0x03}; // Sample Array of Bytes of received simulated
//byte buf[8]; // Buffer array received msg
//========================================================
//bool stxOk = false;
//bool etxOk = false;
char startMarker = 0x02;// STX
char endMarker = 0x03;// ETX
byte numReceived = 0; // number of bytes received from ETX to STX
//========================================================
void setup()
{
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  Serial.begin(transferRate);

  delay(100);

  // Remove Serial prints after debug

  Serial.println(mainPurpose + currentVersion);//
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
void loop() {

  currentMillis = millis();
  if (currentMillis - lastRstMillis >= waitRestart) {
    // save the last time here
    lastRstMillis = currentMillis;
    digitalWrite(ledPin, HIGH); // Busy LED. data should not be sent when OM
    lcd.clear();
    Serial.print("dataCounter: ");
    Serial.println(dataCounter);
    Serial.print("errCounter: ");
    Serial.println(errCounter);
    Serial.println("\n");
    lcd.setCursor(0, 0);
    lcd.print("Data OK: ");
    lcd.print(dataCounter);
    lcd.setCursor(0, 1);
    lcd.print("Data Err: ");
    lcd.print(errCounter);
    delay(pauseDelay); // waiting time to allow reading results of analysis
    dataCounter = 0;// next cycle
    errCounter = 0; // next cycle

    lcd.clear();
    digitalWrite(ledPin, LOW); // Busy LED OFF
    lcd.print("Ready");

    Serial.println(startCode); //Send code to Host
  }
  // continue if waitRestart not overrun

  recvBytesWithStartEndMarkers(); // Get serial Data
  if (newData == true) {
    calcSum(); // Calculate sum
    calculated_cksum = (sum ^ 0xFF) + 1; // This is as same as calculated_cksum = -sum used in V2
    rcvd_cksum = receivedBytes[numReceived - 2];// get received checksum
    sumPlusChksum = sum + rcvd_cksum;
    showAnalysis();// Analysis of data received shown in Serial Out with start TimeOutSerial (Debug)
    verifyChecksum(); // Error Check

    newData = false; // all done ready for next data
  }// Display done new Data
} // End of Loop

//********************************************************
//Fn #01
void lcsWaiting() {
  lcd.clear();        // clear display
  lcd.setCursor(0, 0); // Position Column 0 , Line 1
  lcd.print(fileId + currentVersion);
  lcd.setCursor(0, 1); // Position Column 0 , Line 2
  lcd.print("Baud: " );
  lcd.print(transferRate);//
}
//----------------------------------------------------
//Fn#02
void recvWithEndMarker() {
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

void recvBytesWithStartEndMarkers() {
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
// enter with lcdStr = String to display on 1st line
void displayLcdLine1() {
  lcd.clear();        // clear display
  lcd.setCursor(0, 0);
  lcd.print(lcdStr);
}
//----------------------------------------------------

//Fn#06
void calcSum() {
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
//
//Fn#08
// formerly displayData. Name changed for clarity
// for Debug.
// Serial Display following:
// Received Data Bytes, Sum od received data bytes, Calculated Checksum,
// received checksum byte and Sum of received data bytes and received checksum

void showAnalysis() {
  // Start timer
  lastSerialMillis = millis(); // set previous Serial millis to current millis
  timeOutSerial = true; //flag to start timer to allow Serial delay

  Serial.print("Sum of received data bytes                       = ");
  printHexByte(sum);
  Serial.println();

  //byte calculated_cksum = -sum; // changed V2A now in loop for clarity
  Serial.print("Calculated checksum                              = ");
  printHexByte(calculated_cksum);
  Serial.println();

  // rcvd_cksum = receivedBytes[numReceived - 2];// no calculations here. only display
  Serial.print("Received checksum byte                           = ");
  printHexByte(rcvd_cksum);
  Serial.println();

  // sumPlusChksum = sum + rcvd_cksum;// no calculations here. only display
  Serial.print("Sum of received data bytes and received checksum = ");
  printHexByte(sumPlusChksum);
  Serial.println();
  Serial.println();
}

//---------------------------------------------------------
//Fn#09
void printHexByte(byte x) {
  Serial.print("0x");
  if (x < 16) {
    Serial.print('0');
  }
  Serial.print(x, HEX);
  // Serial.println();
}
//---------------------------------------------------------
//Fn#10
// Serial out number of Bytes & received data string in HEX
void showNewData() {
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
//-------------------------------------------------------------

void verifyChecksum() {
  errorFlag = sum + rcvd_cksum; // Check if OK should be 0
  //Serial.print("Error Flag: "); //Debug
  // Serial.println(errorFlag); // Debud
  if (errorFlag != 0) {
    errCounter++;
  }//Error Check done
  else if (errorFlag == 0) {
    dataCounter++; // increment data counter
    // Serial.print("dataCounter: ");
    // Serial.println(dataCounter);
    // lcd.setCursor(0, 1);
    // lcd.print(dataCounter);
  }
}
//====================================================
//*******************UNUSED FUNCTIONS*********************
/*
  //Fn#04A
  void displayRcvdBytesLcdV1() {
  for (int i = 0; i < numBytes; i++)
  {
    printHexByteLcd(receivedBytes[i]);
  }
  }
  //---------------------------------------------------------
  //Fn#04B
  void printHexByteLcd(byte x) {
  if (x < 16)
  {
    lcd.print('0');
  }
  lcd.print(x, HEX);
  }

  //---------------------------------------------------------------

  //Fn#05
  // Display received bytes in HEX in LCD1602 on line specified before entry
  // Note: Starts background timer with millis
  // the duration is specified by variable interval in declarations
  void displayRcvdBytesLcdV2() {
  sprintf(dataString, "%02X%02X%02X%02X%02X%02X%02X%02X",
          receivedBytes[0], receivedBytes[1], receivedBytes[2], receivedBytes[3],
          receivedBytes[4], receivedBytes[5], receivedBytes[6], receivedBytes[7]);
  lcd.print(dataString);
  //lastLcdMillis = millis(); // set previous to current millis
  //timeOutLcd = true; //flag to start timer
  //digitalWrite(ledPin, HIGH); // show that LCD Delay timer is active
  // removed in V2C
  }

  //---------------------------------------------------------

  //Fn#07
  void displayRcvdBytes() {
  Serial.print("Received data bytes : ");
  for (int i = 0; i < numReceived; i++)
  {
    printHexByte(receivedBytes[i]);
    Serial.print("  ");
  }
  Serial.println();
  }
*/

//==========================================================
