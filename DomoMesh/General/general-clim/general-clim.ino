#include "namedMesh.h"

#define   MESH_SSID       "Xiaomi_6B0F"
#define   MESH_PASSWORD   "250814pyS"
#define   MESH_PORT       5555

// Pin del rele
#define relePin 2  

Scheduler     userScheduler; 
namedMesh  mesh;

String nodeName = "general-clim"; 

// funcion que recibe los mensajes del servidor
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  if( msg == "on"){
    digitalWrite(relePin, LOW);  
  } else if(msg == "off") {
    digitalWrite(relePin, HIGH);
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
