// ********IMPORTE DE LIBRERÍAS***********//
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// ********FIN IMPORTE DE LIBRERÍAS***********//

// ********DECLARACIÓN DE VARIABLES***********//
// const char *ssid = "GobantEdu2023";
// const char *password = "";
const char *ssid = "";
const char *password = "";

const char *mqtt_server = "ngiot2023.online";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_pass = "";

long lastMsg = 0;
char msg[25];
bool autom = false;
int temp = 0;
int hums = 0;
int huma = 0;
int humMin = 60;

#define DHTPIN 0
#define HUMSPIN 36
#define DHTTYPE DHT22

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

  if ((String)topic == "humedadMinima")
  {
    int valHumMin = incoming.toInt();
    humMin = valHumMin;
    Serial.print("Humedad minima");
    Serial.println(humMin);
  }
  if (incoming == "on")
  {
    if ((String)topic == "relay_3")
    {
      autom = true;
      client.publish("alerta", "automatico encendido");
    }
    Serial.print(topic);
    if ((String)topic == "relay_1")
    {
      digitalWrite(5, HIGH);
      client.publish("alerta", "Rele 1 encendido");

      Serial.println("Encendido correactamente r");
    }
    if ((String)topic == "relay_2")
    {
      digitalWrite(17, HIGH);
      client.publish("alerta", "Rele 2 encendido");
      Serial.println("Encendido correactamente r");
    }
  }
  if (incoming == "off")
  {
    if ((String)topic == "relay_1")
    {
      digitalWrite(5, LOW);
      client.publish("alerta", "Rele 1 apagado");
      Serial.println("Apagado correactamente r");
    }
    if ((String)topic == "relay_2")
    {
      digitalWrite(17, LOW);
      client.publish("alerta", "Rele 2 apagado");

      Serial.println("Apagado correactamente r");
    }
    if ((String)topic == "relay_3")
    {
      autom = false;
      client.publish("alerta", "automatico apagado");
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

  if (hums <= humMin)
  {
    digitalWrite(5, HIGH);
  }
  else
  {
    digitalWrite(5, LOW);
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
    }
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
      Serial.println(F("Error reading humidity!"));
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

    String to_send = String(temp) + "," + String(hums) + "," + String(huma);
    to_send.toCharArray(msg, 25);
    Serial.print("Publicación del mensaje");
    Serial.println(msg);
    client.publish("values", msg);
  }
}
