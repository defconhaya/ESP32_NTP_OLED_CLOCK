#include <config.h>
#include <Arduino.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

WiFiUDP ntpUDP;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String temp="N/A";


int offset = 3; // timezone+3


NTPClient timeClient(ntpUDP, "ro.pool.ntp.org",0, 60000);



void display_text(String text)
{
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(10, 40);
    tft.fillRect(0, 40, 127, 9, TFT_BLACK);
    tft.setCursor(10, 40);
    tft.print(text);
}


void display_temp(String temp )
{
    String text="Temp: "+ temp;
    tft.setTextColor(TFT_GREENYELLOW);
    tft.setCursor(10, 20);
    tft.fillRect(0, 20, 127, 9, TFT_BLACK);
    tft.setCursor(10, 20);
    tft.print(temp);
}


void callback(char* topic, byte* payload, unsigned int length) 
{
    // sprintf(buffer, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
    // display_text(topic);
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    temp="";
    for (int i = 0; i < length; i++) 
    {
        Serial.print((char)payload[i]);
        temp+=(char)payload[i];
    }
    display_temp(temp);
    Serial.println();

}


void setup() 
{
    // delay(10);
    Serial.begin(115200);
    Serial.println(F("TFT 1.8\" SPI TFT Test!     ")); 

    tft.init();            // initialize LCD
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, 0);
    tft.print(F(" clock"));


    // pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
    
    timeClient.setTimeOffset(offset*3600);

    // pinMode(0,INPUT);
    // digitalWrite(0,HIGH);

    // pinMode(16,OUTPUT);
    // digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
    // delay(50);
    // digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

    // display.init();
    // display.flipScreenVertically();

    WiFi.begin(ssid, password);



    while ( WiFi.status() != WL_CONNECTED ) 
    {
        display_text("CONN");
        Serial.println("CONN");
        delay ( 500 );
    }

    // display.clear();
    display_text("NTP");
    Serial.print("NTP");

    timeClient.begin();

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    while (!timeClient.forceUpdate())
    {
        delay(10);
    }



}

char buffer[5];

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("/sonoff_afara/ds18b20/Temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() 
{
  timeClient.update();

  if (digitalRead(0) == 0)
  {
    offset += 1;
    if (offset > 14)
    {
      offset = -11;
    }
    timeClient.setTimeOffset(offset*3600);
  }

//   display.clear();

  //display second bar
//   display.fillRect(1, 0, 126*timeClient.getSeconds()/59, 2);

  //display time
  sprintf(buffer, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  display_text(buffer);
  display_temp(temp)  ;
  delay(300);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) 
  {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "ping: #%ld", value);
    // Serial.print("Publish message: ");
    Serial.println(msg);
    // client.publish("outTopic", msg);
  }    
}