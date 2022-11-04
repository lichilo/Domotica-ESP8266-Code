#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h >
#include <WiFiManager.h>
#include <Ticker.h>

const char* ssid     = "";      // SSID / dejar en blanco
const char* password = "";      // Password / dejar en blanco
const char* host = "192.168.0.7";//cambiar / poner ip o url sin el "http://" o el "https://"
const int   port = 80; // cambiar si se necesita http = 80; https = 443;


//pines basados en NodeMCU 1.0(ESP12E) del gestor de targetas compatible con nodemcu V3
const int pinA = D0;//16
const int pinB = D1;//05
const int pinC = D3;//00
const int pinD = D4;//02

/*
const int pinE = D5;//14
const int pinF = D6;//12
const int pinG = D7;//13
const int pinH = D8;//15
*/

const int PIRPin = D6;//12


//Sensor DHT22
#include "DHT.h"
#define DHTPIN 13     //pin D7
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321 / cambiar por DHT11 si es necesario
DHT dht(DHTPIN, DHTTYPE);

Ticker ticker;

void setup()
{

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  //pinMode(pinE, OUTPUT);
  //pinMode(pinF, OUTPUT);
  //pinMode(pinG, OUTPUT);
  //pinMode(pinH, OUTPUT);
  pinMode(PIRPin, INPUT);

  // Empezamos el temporizador que hará parpadear el LED hasta que se conecte a wifi
  ticker.attach(0.2, parpadeoLed);
  WiFiManager wifiManager;
  
  //wifiManager.resetSettings();// Descomentar para que cada vez que se reinicie el modulo te pida la wifi a la cual conectarse

  // Cremos AP y portal cautivo y comprobamos si
  // se establece la conexión
  if (!wifiManager.autoConnect("Domotica", "Domotica")) { // WifiNname & Password  cambiar dependidendo de gustos
    ESP.reset();
    delay(300);
  }

  

  ticker.detach();
  digitalWrite(pinD, HIGH);
  String Strssid = WiFi.SSID();
  const char* ssid     = Strssid.c_str();
  String Strpass = WiFi.psk();
  const char* password = Strpass.c_str();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(20);
    Serial.print(".");
  }
  Serial.begin(500000); // cambiar segun nececidad

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Intensidad de señal");
  Serial.println(WiFi.RSSI());

  //iniciamos el sensor de temperatua y humedad
  dht.begin();
  delay(1000);
}


String dataJson;
void loop() {
    WiFiClient client;
    client.setNoDelay(true);
    if (!client.connect(host, port)) {
      Serial.println("Fallo al conectar");
      delay(200);
      return;
    }

    
    String url = "/ESPRelebox.php?hostname=localhost&datebase=id18712551_houseautomationesp&tabla=tablatry3&username=id18712551_houseautomation&password=id18712551"; // cambiar depende archivo.php y credenciales de inicio MySQL
    url += TempSensor();
    url += PIR(); //pir
    Serial.print(url);


    // Enviamos petición al servidor
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");  
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 200) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Leemos la respuesta del servidor
    while (client.available()) {
      dataJson = client.readStringUntil('\r');

      Serial.println(dataJson);
      JsonDeserializeObject(dataJson);
      delay(20);
    }
}


void JsonDeserializeObject(String dataJson) {
  DynamicJsonDocument dataJsonDes(1024) ;
  deserializeJson(dataJsonDes, dataJson);

  String Bool_1 = String(dataJsonDes["Bool_1"]); // cambiar("Bool_x") depende db
  if (Bool_1 == "0" || Bool_1 == "1") {
    if (Bool_1 == "0") {// == a 0 porque es rele
      digitalWrite(pinA, HIGH);
    } else {
      digitalWrite(pinA, LOW);
    }
  }
  
  String Bool_2 = String(dataJsonDes["Bool_2"]); // cambiar("Bool_x") depende db
  if (Bool_2 == "0" || Bool_2 == "1") {
    if (Bool_2 == "0") {
      digitalWrite(pinB, HIGH);
    } else {
      digitalWrite(pinB, LOW);
    }
  }
  
  String Bool_3 = String(dataJsonDes["Bool_3"]); // cambiar("Bool_x") depende db
  if (Bool_3 == "0" || Bool_3 == "1") {
    if (Bool_3 == "0") {
      digitalWrite(pinC, HIGH);
    } else {
      digitalWrite(pinC, LOW);
    }
  }
  
  String Bool_4 = String(dataJsonDes["Bool_4"]); // cambiar("Bool_x") depende db
  if (Bool_4 == "0" || Bool_4 == "1") {
    if (Bool_4 == "0") {
      digitalWrite(pinD, HIGH);
    } else {
      digitalWrite(pinD, LOW);
    }
  }
  dataJson = "";
}


String TempSensor() {
  String tempData = "";
  //Extraemos los datos del sensor DHT22
  float Humedad = dht.readHumidity();
  float Temperatura = dht.readTemperature();
  int ISensacionTermica = round(dht.computeHeatIndex(Temperatura, Humedad, false) * 10);
  int IHumedad = round(Humedad * 10);
  int ITemperatura = round(Temperatura * 10);

  tempData += "&Float_1="; // cambiar depende php
  tempData += ITemperatura;
  tempData += "&Float_2="; // cambiar depende php
  tempData += IHumedad;
  tempData += "&Float_3="; // cambiar depende php
  tempData += ISensacionTermica;

  return tempData;
}

String PIR()  {
  String tempData = "";
  String Variable = "Bool_5"; // cambiar depende db


  tempData += "&Var="; // cambiar depende php
  tempData += Variable;
  tempData += "&Val="; // cambiar depende php
  tempData += String(digitalRead(PIRPin));

  return tempData;
}
