#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <Ticker.h>

// Update these with values suitable for your network.

//
const char* ssid = "SSD";
const char* password = "PASS";

const char* mqtt_server = "x.xx.xxx.xxx";
const char* mqttUser = 0; 
const char* mqttPassword = 0; 
const String MQTT_FLOW_TOPIC = "sensor/fluxo1";
const String MQTT_TOPIC_RESET = "sensor/fluxo1/reset";
char addr = 10; 

float  flow = 0.0f ;
int address = 0;

float mediaAnterior = 0;

float vazao; //variável para armazenar o valor em L/min
float media=0.0f; //variável para tirar a média a cada 1 minuto
String volume;
float somavazao=0; //somar a vazão do minuto
int contaPulso; //variável para a quantidade de pulsos
int i=0; //variável para contagem

void ICACHE_RAM_ATTR incpulso();

/*
-------------------------------------------------
NodeMCU / ESP8266  |  NodeMCU / ESP8266  |
D0 = 16            |  D6 = 12            |
D1 = 5             |  D7 = 13            |
D2 = 4             |  D8 = 15            |
D3 = 0             |  D9 = 3             |
D4 = 2             |  D10 = 1            |
D5 = 14            |                     |
-------------------------------------------------
*/

void ICACHE_RAM_ATTR handleInterrupt();
boolean stateRelay; 
WiFiClient espClient;
PubSubClient client(espClient);


void Read_Data(){
 EEPROM.get(addr,flow);
}
void Save_Data(float value){
  EEPROM.put(addr, value); // Writes the value 3.141592654 to EEPROM
  EEPROM.commit();
}

//void Read_Data(){
//  flow = EEPROM.read(addr);
//}
//void Save_Data(){
//  EEPROM.write(addr, (float) flow);
//  
//  EEPROM.commit();
//}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  pinMode(4, INPUT);
  attachInterrupt(4, incpulso, RISING); //Configura o pino 2 (interrupção 0) para trabalhar como interrupção
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String payloadStr = "";
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }
      Serial.println(payloadStr);
      String topicStr = String(topic);
      

  }


void reconnect() 
{
   while (!client.connected())
   {
    Serial.println("Conectando ao servidor MQTT...");
     if (client.connect("Projeto", mqttUser, mqttPassword ))
     {
      Serial.println("Conectado ao servidor MQTT!");  
     }else
     {
      Serial.print("Falha ao conectar ");
      Serial.print(client.state());
      delay(2000);
     }
   }
    client.subscribe(MQTT_TOPIC_RESET.c_str()); 
   
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(2560);
  
  Read_Data();


  delay(250);
  
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

 Serial.print("Valor salvo: "); //Imprime L/min
 Serial.println(flow); //Imprime o valor da média
  media =  flow;
  
   Serial.print("Valor salvo media: "); //Imprime L/min
 Serial.println(media); //Imprime o valor da média
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

    contaPulso = 0; //zera a variável para contar os giros por segundo
  sei (); // Habilita a interrupção
  delay (1000); // Aguarda 1 segundo
  cli (); // desabilita interrupção

  vazao = contaPulso / 7.05; //converte para L/min
  somavazao =somavazao+vazao; //soma a vazao para o cálculo da média
  i++;

  Serial.print(vazao); //Imprime na seria o valor da vazão
  Serial.print(" L/min (entrada)- "); //Imprime L/min
  Serial.print(i); //Imprime a contagem i (segundos)
  Serial.println("s"); // Imprime s indicando que está em segundos
  

  if(i==10)
 {
   media = flow + somavazao/60; //Tira a média dividindo por 60
   
   volume = media;
    char message[58];
   String str(media);
   str.toCharArray(message, 58);
   Serial.print(message);
   
   if((media != mediaAnterior)){
   client.publish(MQTT_FLOW_TOPIC.c_str(), message);

   mediaAnterior = media;
   flow = media;
   Save_Data(flow);
   }
   Serial.print("\nVolume acumulado = "); //Imprime a fase média por minuto=
   Serial.print(media); //Imprime o valor da média
     Serial.print("\nVolume Enviado = "); //Imprime a fase média por minuto=
   Serial.print(message); //Imprime o valor da média

   
   Serial.print(" L (entrada) - "); //Imprime L/min
   i=0; //zera a variável i para uma nova contagem
   Serial.println("\n\nInicio\n\n"); //imprime início indicando que a contagem iniciou
}
}
void incpulso ()
{
    contaPulso++; //Incrementa a variável de contagem dos pulsos
}
  
