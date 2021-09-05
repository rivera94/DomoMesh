#include "namedMesh.h"

// Nombre y clave de la red
#define   MESH_SSID       "Xiaomi_6B0F"
#define   MESH_PASSWORD   "250814pyS"
#define   MESH_PORT       5555

// Pines de conexion de los reles
#define subirPin 4
#define bajarPin 2  


Scheduler     userScheduler;
namedMesh  mesh;

String nodeName = "cocina-persiana"; // Nombre del ESP32

// Funcion para recibir mensajes del servidor
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  if( msg == "subir"){
    digitalWrite(bajarPin, HIGH);  
    digitalWrite(subirPin, LOW);
  } else if(msg == "bajar"){
    digitalWrite(bajarPin, LOW);  
    digitalWrite(subirPin, HIGH);
  } else if(msg == "off"){
    digitalWrite(bajarPin, HIGH);  
    digitalWrite(subirPin, HIGH); 
  }
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
  pinMode(subirPin, OUTPUT);
  pinMode(bajarPin, OUTPUT);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION); 

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setName(nodeName);  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  mesh.update();
}
