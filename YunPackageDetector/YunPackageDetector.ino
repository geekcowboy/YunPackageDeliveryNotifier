/////////////////////////////////////////////////////////////////////////////////
// File:  YunPackageDetector.ino
// Author: Mike Parks  http://greenshoegarage.com
// Version: 1.0
// Date Last Revised:  17JAN2014
// Purpose:  A sketch that  detects a change in force exerted
// onto a Force Sensitive Resistor (FSR) connected in a voltage
// divider arrangement on analog input pin 0.
// When force exceeds a threhsold value the Yun does the following:
// 1) Turns on LED on pin 13.
// 2) Flashes message to Console
// 3) Takes a snapshot with a webcam connected to USB port
// 4) Emails copy of photo with dlivery confirmation
// 5) Appends a date/time stamp and sensor reading to Google Drive Spreadsheet
// Also appends the Spreadsheet when force detected drops back to 0.
//
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Include needed header files.
// You will need to create TembooAccount.h
// and Google.h with your own account credentials.
///////////////////////////////////////////////////////////////
#include <Console.h>
#include <Temboo.h>
#include "TembooAccount.h"
#include "Google.h"

///////////////////////////////////////////////////////////////
// Declare needed global variables
///////////////////////////////////////////////////////////////
int sensorVal = 0;             // Stores FSR reading from A0
int threshold = 100;           // Value sensorVal must exceed to trigger notifications
unsigned long prevTime = 0;    // Tracks last time FSR was checked
unsigned long interval = 3000; // Time in milliseconds between readings
bool isDelivered = false;      // Tracks if package has already been delivered
String result = "";            // Placeholder for date/time stamp string

const int ledPin = 13;         // LED on Pin 13
const String SPREADSHEET_TITLE = "yuntest";  // Google Drive Spreadsheet name
const String analogPin = "A0"; // Analog input pin connected to FSR


void setup() 
{
  pinMode(ledPin, OUTPUT);      // Set LED pin as output
  digitalWrite(ledPin, LOW);    // Turn LED off
  Bridge.begin();               // Start communiction to Linino side of Yun
  Console.begin(); 
  while (!Console)
  {
    ; // wait for Console port to connect.
  }
  Console.println("You're connected to the Console!!!!");  // Notify of successful connection to Linino
  Console.println("Waiting for delivery...");
}

void loop() 
{
  if (millis() > prevTime+interval)          //If 3 seconds has elapsed since last reading
  {
    sensorVal = analogRead(analogPin);      // Read the pin attached to FSR
    delay(100);
    //To view FSR reading for debug purposes, uncomment next two lines
    //Console.print("Sensor Reading: ");
    //Console.println(sensorVal);
    prevTime = millis();                    //Update time read
    
    // if FSR reading exceeds threshold value
    if (sensorVal > threshold)    
    {
      // AND no previsus delivery has been noted...
      if (!isDelivered)
      {
        // Turn LED On
        digitalWrite(ledPin, HIGH);
        
        // Print date/time stamp and FSR reading to Console
        Console.print(getTimeStamp());
        Console.println(": Package delivered.");
        Console.print("Sensor Reading: ");
        Console.println(sensorVal);
        
        // Request Linino execute fswebcam to take a photo
        Process takePhoto;
        takePhoto.begin("fswebcam");
        takePhoto.addParameter("-c");
        takePhoto.addParameter("/mnt/sda1/webcam/fswebcam.conf");  // Be sure to create a fswebcam.conf file with your desired settings
        takePhoto.run();
        
        // Request Linino to execute Python script for emailing photo
        Process sendEmail;
        sendEmail.runShellCommand("python /mnt/sda1/pgm/deliveryNotify.py");
        
        // Use Temboo Choreo to append data to a Google Drive Spreadsheet
        appendGDocs();
        
        // Notate package is delivered, and let user know notification is complete.
        isDelivered = true;
        Console.println("Notification complete.");
      }
    }
    
    // Sensor value does NOT exceed threshold
    else
    {
      // A delivery was previsuly noted, so package must have been removed
      if (isDelivered)
      {
        isDelivered = false;                    // Note package is no longer on platform
        digitalWrite(ledPin, LOW);              // Turn LED off
        Console.print(getTimeStamp());          // Send message to console
        Console.println(": Package removed.");
        appendGDocs();                          // Append package removal in Google Drive Spreadsheets
        Console.println("Waiting for next delivery...");
      }
    }
  }
}



///////////////////////////////////////////////////////////////
// Function: getTimeStamp()
// Inputs: None
// Returns: String.  
// Purpose:  Return the current date/time stamp as a string.
// Courtesy of: http://arduino.cc/en/Tutorial/YunDatalogger
///////////////////////////////////////////////////////////////
String getTimeStamp() {
  result = "";
  Process time;
  time.begin("date");
  time.addParameter("+%D-%T");  
  time.run(); 

  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }

  return result;
}



///////////////////////////////////////////////////////////////
// Function: getTimeStamp()
// Inputs: None
// Returns: None
// Purpose:  Append date/time stamp and FSR sensor value to
// a Google Drive Spreadsheet as a new row.
// Courtesy of:  https://www.temboo.com/arduino/update-google-spreadsheet
///////////////////////////////////////////////////////////////
void appendGDocs()
{
    // Initialize Choreo
    TembooChoreo AppendRowChoreo;
    AppendRowChoreo.begin();
    
    // Add Temboo account information
    AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);
   
   // Add Google account information
    AppendRowChoreo.addInput("Username", GOOGLE_USERNAME);
    AppendRowChoreo.addInput("Password", GOOGLE_PASSWORD);
    AppendRowChoreo.addInput("SpreadsheetTitle", SPREADSHEET_TITLE);

    // Create CSV data for appending to spreadsheet, remember the Google
    // Spreadsheet needs to have column headers of your choosing
    String rowData = result;
    rowData += ",";
    rowData += sensorVal;
    AppendRowChoreo.addInput("RowData", rowData);

    // Select Choreo and execute
    AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");
    unsigned int returnCode = AppendRowChoreo.run();

    // If CHoreo has executed successfully
    if (returnCode == 0) 
    {
      Console.println("Successfully appended data to GDocs.");
    } 
    
    // There was a problem executing the Choreo
    else 
    {
      while (AppendRowChoreo.available()) 
      {
        char c = AppendRowChoreo.read();
        Console.print(c);
      }
    }

    AppendRowChoreo.close();
}

