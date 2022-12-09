# Problema03-MQTT

### Integrantes: Daniel Costa, Diego Silva e Thiago Menezes

## 1. Introdução 

## 2. Ambiente e Ferramentas

Para trabalhar com o nodeMCU foi utilizado o `Arduino IDE` e para os arquivos da interface remota(HTML,CSS,Javacript) e Raspberry PI Zero usamos o `Visual studio code`. São remanescentes do projeto anterior, o display lcd Hitachi HD44780U para a exibição dos dados.os botões para controle do intervalo de tempo e os sensores digitais simulados por um push button e o analógico por um potenciômetro, ambos conectados ao nodeMCU(ESP8266), informações sobre as bibliotecas e funções que tambem foram utilizadas no problema 2 podem ser lidas no repositório do mesmo : https://github.com/MI-SD-TEC499/Problema02-Sensores/edit/main/README.md.

## 3. Desenvolvimento

### 3.1 Fluxo de funcionamento do projeto

O projeto gira em torno da comunicação da interface remota e da nodeMCU com broker : 

![image](https://user-images.githubusercontent.com/111393549/206805086-a4084bf9-8318-4a09-b39b-5a604e94e8b6.png)

O display LCD presente na raspberry pi exibe os dados recebidos pela nodeMCU através da UART e envia os dados referentes ao intervalo de atualização, a interface remota e a nodeMCU se conectam com o broker, inicialmente realizando uma inscricão e depois publicando/recebendo atualizações no tópico.

### 3.2 NodeMCU (ESP8266)

O módulo da NodeMCU continua com as funções do projeto 2(leitura dos botões e potenciômetro alem do envio/recebimento de dados via uart), a novidade é a adição da 
comunicação com o `broker` :
```c
#include <PubSubClient.h>
cconst char mqtt_broker = "10.0.0.101";    //Host do broket
const chartopic = "pbl3/envia";            //Topico a ser publicado
const char subtopic = "pbl3/recebe";       //Topico a ser subscrito 
const charmqtt_username = "aluno";         //Usuario
const char mqtt_password = "@luno123";     //Senha
const int mqtt_port = 1883;                //Porta
```
Devido aos problemas encontrados com a biblioteca `mosquitto` , foi definido que a NodeMCU funcionaria como "ponte" entre a raspberry Pi e o broker 
(usamos a uart para a troca de informações entre o SBC e o NodeMcu), para a implementação do MQTT foi utilizada
a biblioteca `PubSubClient`, primeiro criamos um objeto do tipo PubSubClient :

```c
PubSubClient client(espClient);
```

Depois nos conectamos com o broker usando as credenciais demonstradas no código acima(IP,Usuario,Senha e Porta) e escolhemos um tópico,
em seguida durante a execução do `Setup` usamos o `connectMQTT` que vai realizar a comunicação com o broker usando o host definido anteriormente.
Com a conexão concluida podemos escolher um tópico para se inscrever, e publicar uma mensagem :
```c
client.publish(topic, "{teste123,113007042022}"); 
client.subscribe(topic);
```
Tanto `client.subscribe` quanto `client.publish` são da biblioteca `pubsubclient.h` . Vale lembrar que para receber as atualizações nos tópicos inscritos, usamos a função `client.loop`.
### 3.2 Raspberry pi zero

A raspberry PI continua usando o display LCD para a exibição dos dados recebidos da NodeMCU(estado atual do botão, valor do potenciômetro e intervalo de tempo), os valores são exibidos de forma sequencial e são atualizados com base no delay(em segundos), que pode ser alterado pelo usuario utilizando os botões para dimunuir/aumentar o tempo do intervalo.

``c
comando = 0x08;
lcdPosition(lcd,0,0);
send_to_node(comando);
sleep(2);
receive_from_node();
sleep(2);
lcdPosition(lcd,0,7);
comando = 0x03;
send_to_node(comando);
sleep(2);
receive_from_node();           
sleep(2);
```
O código acima mostra o formato de exibição dos dados no display LCD.

### 3.3 Interface remota


