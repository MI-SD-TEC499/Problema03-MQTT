#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <iostream>

#define analogPin A0 // pinagem do potenciômetro

const char* ssid = "INTELBRAS"; //nome da rede utilizada
const char* password = "Pbl-Sistemas-Digitais"; //senha da rede utilizada

// MQTT Broker
const char *mqtt_broker = "10.0.0.101";  //Host do broket
const char *topic = "pbl3/envia";            //Topico a ser publicado
const char *subtopic = "pbl3/recebe";   //Topico a ser subscrito 
const char *mqtt_username = "aluno";         //Usuario
const char *mqtt_password = "@luno*123";         //Senha
const int mqtt_port = 1883;             //Porta

//Variáveis
bool mqttStatus = 0;

//Objetos
WiFiClient espClient;
PubSubClient client(espClient);

//Prototipos
bool connectMQTT();
void callback(char *topic, byte * payload, unsigned int length);


int poten = 0; // variável para armazenar o valor lido do potenciômetro
int temp = 0; //variável armazenando um valor para simular o sensor de temperatura
int umid = 0; //variável armazenando um valor para simular o sensor de umidade
int tempo = 10; //variável para armazenar o tempo de delay

byte comando;
byte resposta;

void wifi_Setup(){
  Serial.begin(115200); //inicia a comunicação serial

  WiFi.mode(WIFI_STA); //inicia o modo de conexão
  WiFi.begin(ssid, password); //inicia a conexão com a rede
  while (WiFi.waitForConnectResult() != WL_CONNECTED) { //espera a conexão
    delay(5000); 
    ESP.restart(); //reinicia o ESP
  }
  ArduinoOTA.setHostname("ESP-10.0.0.109"); //define o nome do host

  ArduinoOTA.begin(); //inicia o OTA
}

void setup() {

  pinMode(D1,INPUT); //pinagem do botão 1
  pinMode(D2,INPUT); //pinagem do botão 2
  pinMode(LED_BUILTIN, OUTPUT); //pinagem do led da nodeMCU
  
  wifi_Setup(); //configuração da rede wifi
  
  Serial.begin(9600); //inicia a comunicação serial

  mqttStatus =  connectMQTT(); //conecta ao broker


}



void loop() {
  ArduinoOTA.handle(); //verifica se há atualizações

  if(digitalRead(D1)==0){ //verifica se estar ocorrendo modificação no botão D1
    temp = 1; //incrementando o valor da temperatura
    delay(50);
  }
  if(digitalRead(D2)==0){ //verifica se estar ocorrendo modificação no botão D2
    umid = 1; //incrementando o valor da umidade
    delay(50);
  }

  if (Serial.available()>0){ //caso tenha dados sendo recebidos
      comando = Serial.read(); //leitura do bit recebido
      delay(50); 

   }
  static long long pooling  = 0;
    //verifica conexao com o broker

  delay(tempo);
   client.loop();    
   switch (comando){ 
      case 0x01: //caso solicite o valor da umidade
         resposta = 0x03;
         Serial.write(resposta); //enviando o vetor de resposta para a raspberry
         Serial.print(umid);

      break;
      case 0x02: //caso solicite o valor da temperatura
         resposta = 0x02;   
         Serial.write(resposta); //enviando o vetor de resposta para a raspberry
         Serial.print(temp);

      break;
      case 0x03: //caso solicite a situação da NodeMCU
         resposta = 0x00;
         Serial.write(resposta); //enviando o vetor de resposta para a raspberry

      break;
      case 0x04: //caso solicite o valor do potênciometro
         poten = analogRead(analogPin); //leitura do valor do potênciometro
         resposta = 0x01;
         Serial.write(resposta); //enviando o vetor de resposta para a raspberry
         Serial.print(poten);

      break; 
      case 0x05: //caso solicite para desligar a led da NodeMCU
         resposta = 0x04;
         digitalWrite(LED_BUILTIN, HIGH); //desligando a led (funcionamento em lógica inversa)
         Serial.write(resposta);

      break;
      case 0x06: //caso solicite para ligar a led da NodeMCU
         resposta = 0x05;
         digitalWrite(LED_BUILTIN, LOW); //ligando a led (funcionamento em lógica inversa)
         Serial.write(resposta);
      break;
      case 0x07:
        if(mqttStatus){
          char monitoring[20];

          digitalWrite(LED_BUILTIN, LOW); //ligando a led (funcionamento em lógica inversa)
          resposta = 0x08;
          Serial.write(resposta); //enviando o vetor de resposta para a raspberry
          delay(2000);
          //Serial.print(umid);
          delay(500);   
          if (millis() > pooling +1000){
            pooling = millis();
            sprintf(monitoring, "%d;%d;%d", umid, temp, poten); //valores que serão enviados para o tópico
            // Fazer um sprintf com os valores recebidos do monitoramento nessa publicação aqui
            client.publish(topic, monitoring); //publica a mensagem no tópico
            //client.publish(topic, "{teste123,113007042022}");
          }
        }
      case 0x08:
        if(Serial.available()>0){ //caso tenha dados sendo recebidos
          tempo = Serial.read(); //leitura do bit recebido
        }
        resposta = 0x07;
        Serial.write(resposta); //enviando o vetor de resposta para a raspberry
        Serial.print(tempo);
      break;

   }
   umid = 0;
   temp = 0;
}

bool connectMQTT() {
  byte tentativa = 0; //quantidade de tentativas de conexão
  client.setServer(mqtt_broker, mqtt_port); //configura o broker
  client.setCallback(callback); //configura a função de callback

  do {
    String client_id = "aluno";
    
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) { //conecta ao broker
      //Serial.println("Exito na conexão:");

      //Serial.printf("Cliente %s conectado ao broker\n", client_id.c_str());
    } else { //caso não consiga conectar
      /*Serial.print("Falha ao conectar: ");
      Serial.print(client.state());
      Serial.println();
      Serial.print("Tentativa: ");
      Serial.println(tentativa);
      delay(2000);
      */
    }
    tentativa++; //incrementa a quantidade de tentativas
  } while (!client.connected() && tentativa < 5);//enquanto não estiver conectado e a quantidade de tentativas for menor que 5
  if (tentativa < 5) { //caso tenha conseguido conectar
    // publish and subscribe   
    client.publish(topic, "{teste123,113007042022}"); //publica a mensagem no tópico
    client.subscribe(subtopic); //se inscreve no tópico

    return 1;
  } else {

    Serial.println("Não conectado");    
    return 0;
  }
}

void callback(char *topic, byte * payload, unsigned int length) { //função de callback
  Serial.print("Message arrived in topic: "); 
  Serial.println(topic);
  char monitoring[20];
  char interval[20];
  //int interval_time;

  
  
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
    // Esses leds são apenas para verificação de que a mensagem tá chegando no tópico
    // client.publish("pbl3/teste", (char*) payload[i]);
    digitalWrite(LED_BUILTIN, LOW); //ligando a led (funcionamento em lógica inversa)
    delay(1000); 
    digitalWrite(LED_BUILTIN, HIGH); //desligando a led (funcionamento em lógica inversa)
  }

  // Alterar essa variável pra utilizar o valor recebido no payload
  tempo = (int) payload[0];
  // Rever lógica com while(true)
  for(int i = 0; i < 5; i++) {
    //delay(interval_time * 1000);
    //sprintf(interval, "Intervalo: %d", 5);
    sprintf(monitoring, "%d;%d;%d", umid, temp, poten); //valores que serão enviados para o tópico
    // Fazer um sprintf com os valores recebidos do monitoramento nessa publicação aqui
    client.publish(topic, monitoring); //publica a mensagem no tópico
    //client.publish("pbl3/teste", interval);
  }
  Serial.println();
  Serial.println("-----------------------");
}

  
