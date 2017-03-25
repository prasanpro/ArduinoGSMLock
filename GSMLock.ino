 /*
Actuate Keypad lock V2

Upon the start of the program, the program send status of the lock. Thereafter is starts sending the GPS location to custom TCP server.
The program takes inputs from 4x4 matrix keypad and activates a relay on successful entry of secret code. It does the same for SMS input having
same combination of characters. The lock relay activated again with keypad input only with * or # input. 

current set password = 787814

Programmed by: Prasan Dutt
Organization: Vajra Infratech Pvt. Ltd.
*/


#include <Keypad.h>
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
SoftwareSerial mySerial(A4, A3); //Initialize SoftwareSerial, and tell it you will be connecting through pins A4 and A3
#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library
Adafruit_GPS GPS(&mySerial); //Create GPS object

String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence
char c;       //Used to read the characters spewing from the GPS module

#include <TimedAction.h>
void readGPS();
TimedAction gpsAction 		=	TimedAction(1000,readGPS);

SoftwareSerial uBlox(A1, A2); //A1-Rx, A2-Tx
int relay = 10;
int lock = 11;
int unLock = 12;
boolean flag = false;

//Define character pointer for storing the secret code
char* secretCode = "787814 ";  //Last space is mandatory to store the enter value 'A' in buffer
int position = 0;

//define the symbols on the buttons of the keypads
const byte ROWS = 4;
const byte COLS = 4;
char Keys[ROWS][COLS] = 
{
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'},
  {'A','B','C','D'}
};
 
byte rowPins[ROWS] = {2,3,4,5};
byte colPins[COLS] = {6,7,8,9};
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(Keys),colPins,rowPins,COLS,ROWS);
 
void setup()
{
	
  Serial.begin(9600); //Baud rate
  uBlox.begin(115200);
  delay(10000);         // Power sequencing delay to stabilize all the peripherals before main program starts
  uBlox.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);          // Delay of 1000 milli seconds or 1 second
  uBlox.println("AT+CMGS=\"+919700780834\"\r"); // Replace x with mobile number
  delay(1000);
  uBlox.println("Lock powered ON");// The SMS text you want to send
  delay(100);
  uBlox.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  uBlox.print("AT+CMGF=1\r");
  delay(5000);
  uBlox.println("AT+CNMI=2,2,0,0,0"); // AT command set for SMS receive mode
  delay(3000);
  uBlox.println("AT+CMGD=1,4");  //Delete all SMS in box
  delay(100);
  pinMode(lock, OUTPUT);
  pinMode(unLock, OUTPUT);
  pinMode(relay, OUTPUT);
  setLocked(true);
  
  //GPRS activation for sending GPS data over TCP
  uBlox.println("AT+CIPMUX=0");
  Serial.println("Selects Single-connection mode");
  delay(5000);
  uBlox.println("AT+CIPSTATUS");
  Serial.println("IP INITIAL");
  delay(5000);
  uBlox.println("AT+CSTT=\"airtelgprs.com\"");
  Serial.println("status=ATTACHED");
  delay(5000);
  uBlox.println("AT+CIPSTATUS");
  Serial.println("IP START");
  delay(5000);
  uBlox.println("AT+CIICR");
  Serial.println("Brings Up Wireless Connection");
  delay(5000);
  uBlox.println("AT+CIPSTATUS");
  Serial.println("IP GPRSACT");
  delay(5000);
  uBlox.println("AT+CIFSR");
  Serial.println("Get Local IP");
  delay(5000);
  uBlox.println("AT+CIPSTART=\"TCP\",\"183.82.112.45\", \"82\"");
  Serial.println("peer-to-peer connection established");
  delay(5000);
}
 
void  loop()
{
  gpsAction.check();
  delay(12);
  uBlox.println("AT+CIPSEND=71");
  delay(200);
  if(NMEA2.length() < 71){
    uBlox.println("000000");
    Serial.println("GPS not acquired");
   }
  else{
    uBlox.println(NMEA2);
    Serial.println("GPS NMEA sent");
   }
  mySerial.end();
  
    
  if (uBlox.available() > 0) 
  {
    flag = uBlox.find("787814");
    if (flag)
    {
      digitalWrite(unLock, HIGH);
      digitalWrite(relay, HIGH);
      digitalWrite(lock, LOW);
      delay(100);
    }
  }
 
char key = customKeypad.getKey();
  if (key == '*' || key == '#') {
    position = 0;
    setLocked(true);
    //Serial.println("\nStart again:");
  }
 
  if (key == secretCode[position]) {
    position++;
    //Serial.print('*');
  }
  if(position == 6){
    if (key == 'A'){
      setLocked(false); 
  }
  }
  delay(50);
  loop();
}

//Lock & Unlock routine
void setLocked(boolean locked)
{
  if (locked)
  {
    digitalWrite(lock, HIGH);
    digitalWrite(unLock, LOW);
  digitalWrite(relay, LOW);
  }
  else 
  {
    digitalWrite(lock, LOW);
    digitalWrite(unLock, HIGH);
  digitalWrite(relay, HIGH);
  }
}

// Get GPS parse data routine
void readGPS(){  //This function will read and remember two NMEA sentences from GPS
  clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out
  while(!GPS.newNMEAreceived()) { //Keep reading characters in this loop until a good NMEA sentence is received
  c=GPS.read(); //read a character from the GPS
  }
GPS.parse(GPS.lastNMEA());  //Once you get a good NMEA, parse it
NMEA1=GPS.lastNMEA();      //Once parsed, save NMEA sentence into NMEA1
while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
NMEA2=GPS.lastNMEA();
}
void clearGPS() {  //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());

}
