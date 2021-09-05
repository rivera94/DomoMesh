#include "namedMesh.h"
#include "DHT.h"
#include <Arduino_JSON.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"

// Definimos los parametros de nuestra Red mesh
#define   MESH_SSID       "Xiaomi_6B0F"
#define   MESH_PASSWORD   "250814pyS"
#define   MESH_PORT       5555

// Pin para el sensor DHT
#define DHTPIN 4  

// Tipo de sensor DHT 
#define DHTTYPE DHT11   // DHT 11

Adafruit_SGP30 sgp;

// Variables para los sensores
float temp;
float hum;
float cO2;
unsigned long previousMillis = 0;   
const long interval = 10000;
      
// Inicializamos el sensor
DHT dht(DHTPIN, DHTTYPE);

Scheduler     userScheduler; 
namedMesh  mesh;

String readings;

String nodeName = "cocina-med"; // Name needs to be unique

void sendMessage() ; 
String getReadings();

Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, &sendMessage);


uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

// Funcion para recoger los datos de los sensores
String getReadings () {
  JSONVar jsonReadings;
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New DHT sensor readings
    hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temp = dht.readTemperature();
  }
  
  sgp.setHumidity(getAbsoluteHumidity(temp, hum));
  cO2 = sgp.eCO2;
  
  jsonReadings["node"] = nodeName;
  jsonReadings["temp"] = temp;
  jsonReadings["hum"] = hum;
  jsonReadings["cO2"] = cO2;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

// Funcion para enviar los mensajes
void sendMessage(){   
    String msg = getReadings();
    mesh.sendBroadcast(msg); 
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  double temp = myObject["temp"];
  double hum = myObject["hum"];
  Serial.print("Node: ");
  Serial.println(node);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}


void setup() {
  Serial.begin(115200);
  dht.begin();
  sgp.begin();
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setName(nodeName);  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {

  mesh.update();
}
