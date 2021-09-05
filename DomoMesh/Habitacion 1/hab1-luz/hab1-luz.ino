#include "namedMesh.h"
#include <Arduino_JSON.h>

#define   MESH_SSID       "Xiaomi_6B0F"
#define   MESH_PASSWORD   "250814pyS"
#define   MESH_PORT       5555

#define relePin 4  


Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;
bool flag = false;
String readings;
float Sensibilidad = 0.250; // Sensibilidad calculada mediante los programas de prueba
float offset=0.066; // Equivale a la amplitud del ruido
float volt = 0 ;
float volt_ant = 0;
float volt_con = 0;
String nodeName = "hab1-luz"; // Nombre del ESP32

void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, &sendMessage);

float get_corriente(){
  
  float voltajeSensor;
  float corriente=0;
  long tiempo=millis();
  float Imax=0;
  float Imin=0;
  while(millis()-tiempo<1000) //realizamos mediciones durante 0.5 segundos
  { 
    voltajeSensor = analogRead(A0) * (5.0 / 4095.0);//lectura del sensor
    corriente=0.9*corriente+0.1*((voltajeSensor-2.527)/Sensibilidad); //Ecuación  para obtener la corriente
    if(corriente>Imax)Imax=corriente;
    if(corriente<Imin)Imin=corriente;
  }
  return(((Imax-Imin)/2)-offset);
}

void sendMessage(){   
    String msg = readings;
    mesh.sendBroadcast(msg); 
}

// Funcion que controla los mensajes recibidos del servidor
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  String cad = msg.c_str(); 
  if( cad == "change"){
    if(!flag){
      digitalWrite(relePin, HIGH);  
    } else {
      digitalWrite(relePin, LOW);
    }
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
  pinMode(relePin, OUTPUT);
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

  JSONVar jsonReadings;
  volt_con = volt;
  volt = get_corriente();
  if ((volt - volt_con) > 0.04) {
    flag = true;
    volt_ant = volt_con;
  } else if ((volt - volt_ant) < 0.01){
    flag = false;
  }
  jsonReadings["node"] = nodeName;
  jsonReadings["power"] = flag;
  readings = JSON.stringify(jsonReadings);
  sendMessage();
}
