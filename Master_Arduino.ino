// Include the required Wire library for I2C
#include <AceWire.h>
#include <Wire.h>
#include "DHT.h"
#include <SPI.h>
#include <Ethernet.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor PIN 2
#define DHTTYPE DHT11   // DHT 11

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

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

DHT dht(DHTPIN, DHTTYPE);
int smokeA0 = A0;
// Your threshold value
int sensorThres = 100;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement
String sensorJSON = "";


void setup() {
  // Start the I2C Bus as Master
  Wire.begin(); 
  Serial.begin(9600);
  dht.begin();
  pinMode(smokeA0, INPUT);
  
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
}


void loop() {
   // read humidity
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  int t = dht.readTemperature();

  int analogSensor = analogRead(smokeA0);
  
  String tt =(String)t;
  String hh = (String)h;
  String C = (String)analogSensor;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) 
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(5000);
    return;
  }
    
  Serial.print(F(" Hum: "));
  Serial.print(h);
  Serial.print(F("  Temp: "));
  Serial.print(t);
  Serial.print("  CO2: ");
  Serial.print(analogSensor);
  Serial.println();

 sensorJSON = "{\"baby_id\":1,\"temperature_reading\":"+tt+",\"humidity_reading\":"+hh+",\"oxygen_reading\":90,\"co2_reading\":"+C+",\"heartbeat_reading\":99}";

  if (client.connect("192.168.1.133",47543)) // REPLACE WITH YOUR SERVER ADDRESS
  { 
    client.println("POST /api/readings/Add HTTP/1.1"); // HTTP POST TO /results
    client.println("Content-Type: application/json"); // DATA TYPE
    client.println("Accept: */*");
    client.println("Host:192.168.1.133:47543"); // SERVER ADDRESS HERE TOO
    client.println("Connection: Close");
    client.print("Content-Length: "); 
    client.println(sensorJSON.length());
    client.println();
    client.println(sensorJSON); 
  } 
  if (client.connected()) 
  { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }

  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(h);             // sends humidity 
  Wire.write(t);             // sends temperature
  Wire.write(analogSensor);  // sends CO2
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(9, 6);    // request 6 bytes from slave device #8
  while (Wire.available())   // slave may send less than requested
  { 
    char x = Wire.read();    // receive a byte as character
    Serial.print(x);         // print the character
  }
 
}
