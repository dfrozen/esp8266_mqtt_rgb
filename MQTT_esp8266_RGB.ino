
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

const char* host = "esp8266-rgb";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
const char* ssid = "Bestoloch";
const char* password = "511794sinikon";

#define MQTT_SERVER "10.10.100.14"  ///YourMQTTBroker'sIP
const int mqtt_port = 1883;
const char *mqtt_user = "orangepi";
const char *mqtt_pass = "orangepi";
uint32_t ms_button = 0;
const int RED = 12;
const int GREEN = 14;
const int BLUE = 4;
const int PIN_BUTTON = 2;
int r = 0;
int g = 0;
int b = 0;
boolean button_state = false;



ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient client(wifiClient, MQTT_SERVER, mqtt_port);
#define BUFFER_SIZE 100


String macToStr(const uint8_t* mac){
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
   // if (i < 5){result += ':';}
  }
  return result;
}
void callback(const MQTT::Publish& pub){
  
  String payload = pub.payload_string();
  
  if(String(pub.topic()) == "/home/light/rgb1/rgb")
  {
    //rgb(234,213,98)
    String dataSt = String(payload);
    r = dataSt.substring(dataSt.indexOf('(')+1).toInt();
    g = dataSt.substring(dataSt.indexOf(',')+1,dataSt.lastIndexOf(',')).toInt();
    b = dataSt.substring(dataSt.lastIndexOf(',')+1).toInt();
    r= map(r, 0, 255, 0, 1023);
    analogWrite(RED, r);
    g= map(g, 0, 255, 0, 1023);
    analogWrite(GREEN, g);
    b= map(b, 0, 255, 0, 1023);
    analogWrite(BLUE, b);
    Serial.println(dataSt);
    button_state = true;
    client.publish("home/light/rgb1/status/","lightOn");
    if (r==0 && g==0 && b==0) {client.publish("home/light/rgb1/status/","lightOff");
    button_state = false;}
  }

}
void re_connect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
   Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");
      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      //  if (client.connect((char*) clientName.c_str())) {
      // if (client.connect(MQTT::Connect("arduinoClient2")
      //                   .set_auth(mqtt_user, mqtt_pass)))
      if (client.connect(MQTT:: Connect(clientName.c_str()).set_auth(mqtt_user, mqtt_pass))) {
        Serial.println("\tMQTT Connected");
        client.set_callback(callback);
        
        client.subscribe("/home/light/rgb1/rgb");
        client.subscribe("home/light/rgb1/status");
        
      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}  
void setup() {
 pinMode (12,OUTPUT);
 pinMode (14,OUTPUT);
 pinMode (4,OUTPUT);
 pinMode (2,INPUT_PULLUP);
 
 Serial.begin(115200);
// Status=digitalRead(Button);
// Serial.println(Status);
 re_connect();
  MDNS.begin(host);
httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void loop(void) {
  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {re_connect();}
  //maintain MQTT connection
  //MUST delay to allow ESP8266 WIFI functions to run
 delay(200);  
  httpServer.handleClient();

 //boolean flag = true;  
// Фиксируем нажатие кнопки
    uint32_t ms = millis();
// Фиксируем нажатие кнопки
   if( digitalRead(PIN_BUTTON) == LOW && !button_state && ( ms - ms_button ) > 50 ){
      button_state = true;
    Serial.println("Press key");
    analogWrite(RED, 100);
    analogWrite(GREEN, 100);
    analogWrite(RED, 100);
    client.publish("home/light/rgb1/status/","lightOn");
    client.publish("/home/light/rgb1/rgb","(100,100,100)");
      ms_button    = ms;
   }
// Фиксируем отпускание кнопки
   if( digitalRead(PIN_BUTTON) == LOW && button_state && ( ms - ms_button ) > 50 ){
      button_state = false;   
      Serial.println("Press key");  
    analogWrite(RED, LOW);
    analogWrite(GREEN, LOW);
    analogWrite(RED, LOW);  
    client.publish("home/light/rgb1/status/","lightOff");
    client.publish("/home/light/rgb1/rgb","(0,0,0)");
      ms_button    = ms;
   }
 
 
     
  client.loop();
}
  

