// ********IMPORTE DE LIBRERÍAS***********//
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HTTPClient.h>

// ********FIN IMPORTE DE LIBRERÍAS***********//

// ********DECLARACIÓN DE VARIABLES***********//
// const char *ssid = "GobantEdu2023";
// const char *password = "";
const char *ssid = "Oszanet_meneses";
const char *password = "jorge890";

const char *mqtt_server = "automaticiot.cloud";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client";
const char *mqtt_pass = "121212";
 String macAddress = "";
  String macAddressWeb = "";
long lastMsg = 0;
char msg[25];
char msg2[25];
bool autom = false;
int temp = 0;
int hums = 0;
int huma = 0;
int humMin = 60;
bool enviar=false;
#define DHTPIN 0
#define HUMSPIN 36
#define DHTTYPE DHT11

// ********FIN DECLARACIÓN DE VARIABLES***********//

// ********CLIENTE WI FI Y MQTT CLIENTE***********//
WiFiClient espClient;

PubSubClient client(espClient);
// ********FIN CLIENTE WI FI Y MQTT CLIENTE***********//

// ********DELCARACIÓN DE FUNCIONES***********//
DHT_Unified dht(DHTPIN, DHTTYPE);
void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void automatico();
// ********DECLARACIÓN DE FUNCIONES***********//

void setup()
{
  digitalWrite(13, OUTPUT);

  Serial.begin(115200);
  randomSeed(micros());
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(17, OUTPUT);
   uint8_t mac[6];
   WiFi.macAddress(mac);  
  for (int i = 0; i < 6; ++i) {
    macAddress += String(mac[i], 16);
    if (i < 5) {
      macAddress += ':';
    }
  }
}

// ********CONEXIÓN DEL WIFI A LA RED***********//
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando a");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a WIFI");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}

// ********FIN CONEXIÓN DEL WIFI A LA RED***********//

// ********RECEPCIÓN DE MENSAJES DEL BROKER***********//
void callback(char *topic, byte *payload, unsigned int length)
{
  String incoming = "";
  Serial.print("Mensaje recibido desde ->");
  Serial.print(topic);
  Serial.println("");

  for (int i = 0; i < length; i++)
  {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje ->" + incoming);
    if((String)topic=="mac"){
      macAddressWeb=(String)incoming;
      Serial.println(macAddressWeb);
      Serial.println(macAddress);
      delay(500);
      enviar=true;
      if(macAddressWeb=="desconectado" || macAddress!=macAddressWeb){
        enviar=false;
        Serial.println("DESCONECTADO");
      }else{
        if(macAddress==macAddressWeb){
          enviar=true;
        }
        
      }
      
    }
     Serial.print(topic);
      if (incoming == "on")
    {
    if ((String)topic == "relay_3")
    {
        if(macAddress==macAddressWeb){
           autom = true;
          String alertMac=String("Modo auto encendido") + ","+String(macAddress);
          alertMac.toCharArray(msg2,50);
          client.publish("alerta",msg2);    
        }
     
    }
    if ((String)topic == "relay_1")
    {
       
     if(macAddress==macAddressWeb){
      digitalWrite(5, HIGH);
      String alertMac=String("Rele 1 encendido") + ","+String(macAddress);
      alertMac.toCharArray(msg2,50);
       client.publish("alerta",msg2);      
      Serial.println("Encendido correactamente r");
     }
      
     
    }
    if ((String)topic == "relay_2")
    {
        if(macAddress==macAddressWeb){
            digitalWrite(17, HIGH);
       String alertMac=String("Rele 2 encendido") + ","+String(macAddress);
      alertMac.toCharArray(msg2,50);
       client.publish("alerta",msg2);    
      Serial.println("Encendido correactamente r");
        }
    
    }
  }
  if (incoming == "off")
  {
    if ((String)topic == "relay_1")
    {
      if(macAddress==macAddressWeb){
      digitalWrite(5, LOW);     
      Serial.println("Apagado correactamente r");
      String alertMac=String("Rele 1 apagado") + ","+String(macAddress);
      alertMac.toCharArray(msg2,50);
       client.publish("alerta",msg2);      
      }
    }
    if ((String)topic == "relay_2")
    {
       if(macAddress==macAddressWeb){
          digitalWrite(17, LOW);
        String alertMac=String("Rele 2 apagado") + ","+String(macAddress);
      alertMac.toCharArray(msg2,50);
       client.publish("alerta",msg2);    
      Serial.println("Encendido correactamente r");
       }
    

     
    }
    if ((String)topic == "relay_3")
    {
        if(macAddress==macAddressWeb){
      autom = false;
          String alertMac=String("Modo auto apagado") + ","+String(macAddress);
          alertMac.toCharArray(msg2,50);
          client.publish("alerta",msg2);    
        }
     
    }
  }
    
// REVISAR PROGRAMACIÓN DE HUMEDAD MÍNIMA
  if ((String)topic == "humedadMinima")
  {
    char datoCapturado[20]; 
     char macHumMin[30]; 
     if(macAddress==macAddressWeb){
      char valHumMin[27];
      incoming.toCharArray(valHumMin,30);
       char tokens[2][30];

      char *token =strtok(valHumMin,",");
      int i=0;
      while(token!=NULL && i< 2 ){
        strcpy(tokens[i], token);
         Serial.println(tokens[i]);
         token = strtok(NULL, ",");
         i++;
      }
       strcpy(datoCapturado, tokens[0]);
       strcpy(macHumMin, tokens[1]);
    
       String newMac=macHumMin;
if(macAddress==newMac){
   humMin=atoi(datoCapturado);
    Serial.print("Humedad minima");
    Serial.println(humMin);
}
  
     }
   
  }
 
}
// ******** FIN RECEPCIÓN DE MENSAJES DEL BROKER***********//

// ******** RECONONEXIÓN AL  BROKER***********//
void reconnect()
{

  while (!client.connected())
  {
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = "esp32_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println("Conectado!");
      // Nos suscribimos
      client.subscribe("relay_1");
      client.subscribe("relay_2");
      client.subscribe("relay_3");
      client.subscribe("relay_4");
      client.subscribe("humedadMinima");
      client.subscribe("mac");
    }
    else
    {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");

      delay(5000);
    }
  }
}

void automatico()
{
    Serial.println(hums);
    Serial.println(humMin);
   
  if (hums <= humMin)
  {
    digitalWrite(5, HIGH);
    Serial.println("Riego activado");
  }
  else
  {
    digitalWrite(5, LOW);
     Serial.println("Riego apagado");
  }
}

// ******** FIN RECONONEXIÓN AL  BROKER***********//
void loop()
{
  
  if (!client.connected())
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      setup_wifi();
    }
    reconnect();
  }

  client.loop();
  long now = millis();
  if (now - lastMsg > 500)
  {
    lastMsg = now;
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
      Serial.println(F("Error reading temperature!"));
   
    }
    else
    {
      temp = event.temperature;
      Serial.print(F("Temperature: "));
      Serial.print(event.temperature);
      Serial.println(F("°C"));
      // Serial.println(macAddressWeb);
      // Serial.println(macAddress);
    }
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
      Serial.println(F("Error reading humidity!"));
      // Serial.println(macAddressWeb);
      // Serial.println(macAddress);
    }
    else
    {
      Serial.print(F("Humidity: "));
      Serial.print(event.relative_humidity);
      Serial.println(F("%"));
      huma = event.relative_humidity;
    }
   
    if (autom)
    {
      automatico();
    }
    hums = analogRead(HUMSPIN);
    hums = map(hums, 4095, 0, 0, 100);
   
 

    String to_send = String(temp) + "," + String(hums) + "," + String(huma)+ "," + macAddress;
    to_send.toCharArray(msg, 30);
    Serial.print("Publicación del mensaje");
    if(enviar){
          client.publish("values", msg);
              Serial.println(msg);
    }

  }
}
