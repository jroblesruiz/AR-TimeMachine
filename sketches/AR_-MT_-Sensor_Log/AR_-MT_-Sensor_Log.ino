
#include <ESP.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
#include <RTClib.h>
RTC_DS1307 RTC;

// Wi-Fi Settings
WiFiClient client;

// Punishbox.com Settings
String APIKey = "v5796F847AD8C763";
const char* server = "api.pushingbox.com";

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
const int postingInterval = 3 * 1000; // post data every 20 seconds
const int SENSOR = 0;                       // Pin del sensor de corriente

DateTime now;

void setup() {

  wifiMulti.addAP("RGNET", "apocrifa11manana");
  wifiMulti.addAP("JRR-iPhone", "3314665169");
  wifiMulti.addAP("Proyectos", "12345@12345");
  wifiMulti.addAP("Laser_Express", "Laser2505Express!");

  Serial.begin(115200);		      // Initialize serial communications with the PC
  while (!Serial);		          // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  emon1.current(SENSOR, 4.1);   // Current: input pin, calibration.

  // Inicio la conexión a la red Wifi
  //WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Conectando al WiFi.");
  while (wifiMulti.run()  != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }  Serial.println();
  Serial.print("Conectado a ");
  Serial.println(WiFi.SSID());
  Serial.println("Intensidad: " + String(WiFi.RSSI()) + "db");
}

void loop() {
  /*
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
    return;
  }
  */
  
  if (!client.connected() && lastConnected)
  {
    Serial.println("...desconectado");
    Serial.println();
    client.stop();
  }

  if (failedCounter > 3 )
  {
    WiFi.status();
  }
  lastConnected = client.connected();


  // Measure Signal Strength (RSSI) of Wi-Fi connection
  long rssi = WiFi.RSSI();

  // Measure IRMS current
  double irms = emon1.calcIrms(1480);  // Calculate Irms only
  Serial.print(irms * 127.0);       // Apparent power
  Serial.print(" ");
  Serial.println(irms, 4);         // Irms

  Serial.print("RSSI: ");
  Serial.println(rssi);

  Serial.print("Mac Address: ");
  Serial.println(WiFi.macAddress());

  Serial.print("Flash Size: ");
  Serial.println(ESP.getFlashChipSize());
   
  Serial.print("Local IP ");
  Serial.println(WiFi.localIP());

  WiFi.printDiag(Serial);
  
  updateData(rssi, irms );

  // wait and then post again
  delay(postingInterval);

}

void updateData(long rssi, double irms )
{

  //Start or API service using our WiFi Client through PushingBox
  if (client.connect(server, 80))
  {

    //Serial.println(String(server) + "/pushingbox?devid=" + APIKey
    //               + "&RSSI=" + String(rssi) + "&IRMS=" + String(irms, 4));

    client.print("GET /pushingbox?devid=" + APIKey
                 + "&RSSI=" + String(rssi) + "&IRMS=" + String(irms, 4) + "&MAC=" + WiFi.macAddress() );
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("User-Agent: ESP8266/1.0");
    client.println("Connection: close");
    client.println();


    lastConnectionTime = millis();

    if (client.connected())
    {
      Serial.println("Connecting to Punishbox...");
      Serial.println();

      failedCounter = 0;
    }
    else
    {
      failedCounter++;

      Serial.println("Connection to Punishbox failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }

  }
  else
  {
    failedCounter++;

    Serial.println("Connection to Punishbox Failed (" + String(failedCounter, DEC) + ")");
    Serial.println();

    lastConnectionTime = millis();
  }

}
