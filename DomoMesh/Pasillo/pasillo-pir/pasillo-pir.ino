#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <Arduino_JSON.h>

#define   MESH_PREFIX     "Xiaomi_6B0F"
#define   MESH_PASSWORD   "250814pyS"
#define   MESH_PORT       5555

#define   STATION_SSID     "Xiaomi_6B0F"
#define   STATION_PASSWORD "250814pyS"

#define HOSTNAME "MQTT_Bridge"

// Pin de conexion del sensor
#define PIRPin 4  

String readings;
String nodeName = "pasillo-pir"; // Nombre del ESP32
bool band;

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);
String getReadings ();

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 31, 196);

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  
  pinMode(PIRPin, INPUT);
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 ); // Definimos el canal 6 para la red wifi y la red mesh
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Definimos este nodo como raiz
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

}

void loop() {
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("painlessMeshClient")) {
      mqttClient.publish("painlessMesh/from/gateway","Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
    } 
  }

  band = digitalRead(PIRPin);
  if(band){
    String msg = getReadings();
    String topic = "painlessMesh/from/" + String(nodeName);
    mqttClient.publish(topic.c_str(), msg.c_str()); 
  }
  
}

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeName;
  jsonReadings["move"] = "on";
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("painlessMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
