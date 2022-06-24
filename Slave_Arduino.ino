#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xDA, 0xEB, 0xFE, 0xEF, 0xDE };

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "192.168.1.133";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial sim(9, 10);

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement
String sensorJSON = "";

int Temp;
int Hum;
int CO2;
int _timeout;
String _buffer;
String number = "+201022245434";
String SMS;

void setup() {
  delay(7000);
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();

  lcd.print("T:");
  lcd.setCursor(9, 0);
  lcd.print("H:");
  lcd.setCursor(0, 1);
  lcd.print("C:");

  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware.");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 47543)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  beginMicros = micros();
  
  _buffer.reserve(50);
  sim.begin(9600);
  

  
  delay(1000);
}

void requestEvent()
{
  Wire.write("Done! ");
}

void receiveEvent(int bytes) {
  Hum = Wire.read();    // read one character from the I2C
  Temp = Wire.read();
  CO2 = Wire.read();
}

void loop() {
  String tt =(String) Temp;
  String hh = (String) Hum;
  String C = (String) CO2;

  Serial.print(F(" Humidity: "));
  Serial.print(Hum);
  Serial.print(F("  Temperature: "));
  Serial.print(Temp);
  Serial.print("  CO2: ");
  Serial.print(CO2);
  Serial.println();
  
  lcd.setCursor(2, 0);
  lcd.print(Temp);
  lcd.setCursor(11, 0);
  lcd.print(Hum);
  lcd.setCursor(2, 1);
  lcd.print(CO2);

  if(((Temp > 38) && (Temp < 35)) || (Hum > 50) || (CO2 > 20))
  {
    SendMessage(); 
    
   sensorJSON = ("{\"doctor_id\":11 ,\"doctor_mobile\":01022245434 ,\"baby_id\":1 ,\"msg_txt\":\"Alert Check Baby Baby_id: 1\"}");
 
 
  if (client.connect("192.168.1.133",47543)) {         // REPLACE WITH YOUR SERVER ADDRESS
    client.println("POST /api/gsm/Add HTTP/1.1"); // HTTP POST TO /results
    client.println("Content-Type: application/json");  // DATA TYPE
    client.println("Accept: */*");
    client.println("Host:192.168.1.133:47543");        // SERVER ADDRESS HERE TOO
    client.println("Connection: Close");
    client.print("Content-Length: "); 
    client.println(sensorJSON.length());
    client.println();
    client.println(sensorJSON); 
    //client.println(); 
  } 
  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  } 
  }
  
  Serial.print(F(" Humidity: "));
  Serial.print(Hum);
  Serial.print(F("  Temperature: "));
  Serial.print(Temp);
  Serial.print("  CO2: ");
  Serial.print(CO2);
  Serial.println();

delay(10000);
}

void SendMessage()
{
  //Serial.println ("Sending Message");
  sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);
  //Serial.println ("Set SMS Number");
  sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(1000);
  SMS = ("Alert!! Check Baby!! baby_id: 1");

  sim.println(SMS);
  delay(100);
  sim.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  _buffer = _readSerial();
}

String _readSerial() 
{
  _timeout = 0;
  while  (!sim.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}
