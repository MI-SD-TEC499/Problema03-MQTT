# Problema03-MQTT

### Integrantes: Daniel Costa, Diego Silva e Thiago Menezes

## 1. Introdução 

## 2. Ambiente e Ferramentas

## 3. Desenvolvimento

### 3.1 Fluxo de funcionamento do projeto

### 3.2 NodeMCU (ESP8266)

O módulo da NodeMCU continua com as funções do projeto 2(leitura dos botões e potenciômetro alem do envio/recebimento de dados via uart), a novidade é a adição da 
comunicação com o `broker`:
```c
#include <PubSubClient.h>
const char *mqtt_broker = "10.0.0.101";    //Host do broket
const char *topic = "pbl3/teste";          //Topico a ser subscrito e publicado
const char *mqtt_username = "aluno";       //Usuario
const char *mqtt_password = "@luno*123";   //Senha
const int mqtt_port = 1883;                //Porta
```
Devido aos problemas encontrados com a biblioteca `mosquitto` , foi definido que a NodeMCU funcionaria como "ponte" entre a raspberry Pi e o broker 
(usamos a uart para a troca de informações entre o SBC e o NodeMcu), para a implementação do MQTT foi utilizada
a biblioteca `PubSubClient`, primeiro nos conectamos com o broker usando as credenciais demonstradas no código acima(IP,Usuario,Senha e Porta) e escolhemos um topico,
em seguida durante a execução do Setup usamos o `connectMQTT` que vai realizar a comunicação com o broker usando o host definido anteriormente.
Com a conexão concluida podemos escolher um tópico para se inscrever, e publicar uma mensagem :
```c
client.publish(topic, "{teste123,113007042022}"); 
client.subscribe(topic);
```
Vale lembrar que para receber as atualizações nos tópicos inscritos, o cliente MQTT deve estar em loop, para isso usamos `client.loop`.

### 3.2 Raspberry pi zero

Continuamos usando o display LCD para a exibição dos dados recebidos através da UART, os valores recebidos são exibidos de forma sequencial em um intervalo de tempo definido pelo usuario,


### 3.3 Interface remota


