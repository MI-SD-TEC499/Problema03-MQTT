# Problema03-MQTT

### Integrantes: Daniel Costa, Diego Silva e Thiago Menezes

## 1. Introdução 
Vários tipos de protocolos de comunicação estão sendo utilizados no mundo atualmente, principalmente no mundo da Internet of Things (IOT),  MQTT é um desses protocolos, sua forma simples e eficiente de distribuição de mensagens acabou o tornando extremamente popular, ele é baseado em clientes que se inscrevem em um tópico para publicar e receber atualizações, servindo como alternativa eficaz ao método cliente/servidor.
Com base nesse conceito alteramos nosso projeto anterior para que seja realizada a implementação do protocolo MQTT, com o objetivo de tornar possível  a visualização de das informações dos sensores para os dispositivos que estejam conectados ao servidor central(broker).


## 2. Ambiente e Ferramentas

Para trabalhar com o nodeMCU(ESP8266) foi utilizado o `Arduino IDE` , para os arquivos da interface remota (HTML,CSS,Javacript) e Raspberry PI Zero usamos o `Visual studio code`. São remanescentes do projeto anterior, o display lcd Hitachi HD44780U para a exibição dos dados, os botões para controle do intervalo de tempo e os sensores digitais simulados por um push button e o analógico por um potenciômetro, ambos conectados ao nodeMCU, informações sobre as bibliotecas e funções que tambem foram utilizadas no problema 2 podem ser lidas no repositório do mesmo, juntamente com as informações de como configurar a IDE para poder realizar a comunicação com o nodeMCU: [Repositório Problema 2](https://github.com/MI-SD-TEC499/Problema02-Sensores/edit/main/README.md).

Todos os arquivos referentes a interfece remota se encontram nesse outro repositório em função de uma melhor organização de códigos: [Interface Remota Problema 3](https://github.com/MI-SD-TEC499/Interface-Web-MQTT)

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

```c
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

Para a criação da interface remota foi utilizada a linguagem de marcação HTML para exibição do dados que foram enviados pela nodeMCU. Para uma melhor organização do projeto, os códigos referentes a exibição remota foi armazenada em outro repositório: [Repositório Interface Remota](https://github.com/MI-SD-TEC499/Interface-Web-MQTT).

No arquivo index.html é onde se encontra a parte de exibição da interface remota, foi possível a criar inputs para que o usuário consiga informar a qual tópico do MQTT ele irá receber as informações, juntamente com um botão para enviar essa informação, além de outros dois inputs, um para estabelecer o tópico de envio dos dados e outro com o valor que dejesa enviar, junto com um botão para mandar essa informação.

```html
  <label for="subscribe">Subscribe</label>
  <input type="text" id="subscribe" />
  <button id="subscribe-button">Send</button>

  <label for="topic">Topic</label>
  <input type="text" id="topic" />
  <label for="message">Send Message</label>
  <input type="text" id="message" />
  <button id="send-button">Send</button>
```

Para a exibição dos dados recebidos da nodeMCU foi utilizada a biblioteca [Chart.js](https://www.chartjs.org/docs/latest/), essa biblioteca cria um gráficos com as informações definidas, no nosso caso sendo a umidade(simulada por um botão), a temperatura(simulada por um botão) e o potênciomentro.

```js
  const linechart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: ['', '', '', '', '', '', '', '', '', ''],
        datasets: [
          {
            label: 'Temperatura',
            data: temperature,
            borderWidth: 1
          },
          {
            label: 'Umidade',
            data: umidity,
            borderWidth: 1
          },
          {
            label: 'Potenciômetro',
            data: potenciometer,
            borderWidth: 1
          },
        ]
      },
      options: {
        scales: {
          y: {
            beginAtZero: true
          }
        },
      }
    });
```

No arquivo index.js é onde se encontra toda a parte de conexão com o MQTT e a nodeMCU. Para a conexão foi utilizada a biblioteca [Eclipse Paho JavaScript Client](https://www.eclipse.org/paho/index.php?page=clients/js/index.php), onde definimos dados em sua classe como o host, a porta utilizada, o username e a senha do broker para que a conexão seja estabelecida com sucesso.

```js
  class PahoHandler {
    //definindo configurações para a conexão ao MQTT
    constructor(host, port, clientId) {
      this.host = host;
      this.port = port;
      this.clientId = clientId;
      this.client = new Paho.MQTT.Client(this.host, this.port, this.clientId);
      this.client.connect({
        userName: "username_exempo",
        password: "senha_exemplo",
        onSuccess: this.onSuccess,
        timeout: 3,
      });
    }
```

Para poder criar a conexão com o broker criamos o objeto do PahoHandler para poder executar as ações:

```js
  const pahoHandler = new PahoHandler('exemplo_IP', exemplo_porta, "clientjs");
```

Com isso temos as funções de subscribe para se inscrever no tópico que foi informado pelo usuário, e temos a função send para enviar a mensagem que o usuário deseja.

```js
  pahoHandler.subscribe(subscribeInput.value);
```

```js
  pahoHandler.send(topicInput.value, messageInput.value);
```

E a função que fica recebendo a mensagem do nodeMCU sempre que ele enviar e atualizando o gráfico com as informações recebidas:

```js
const onMessage = (message) => {
      var payload = message.payloadString;
      splited = payload.split(";"); //separando os valores recebidos no array
      if (umidity.length == 10) {
        umidity.shift();
        temperature.shift();
        potenciometer.shift();
      }
      //adicionando o valor recebido no gráfico
      umidity.push(Number(splited[0]));
      temperature.push(Number(splited[1]));
      potenciometer.push(Number(splited[2]));

      linechart.update(); //atualizando o gráfico
    };
```
## 4. Executando o projeto

### 4.1 NodeMCU

Para que o arquivo `NodeMCU.ino` seja executado corretamente, é necessário que a biblioteca PubSubClient seja adicionada, o restante do processo de execução é o mesmo feito no [problema 2](https://github.com/MI-SD-TEC499/Problema02-Sensores)(autenticação com o login e senha da WIFI e escolha da porta na interface do arduino IDE) vale lembrar que como as informações relacionadas ao broker estão na NodeMCU será preciso mudar os dados no arquivo caso as credenciais do broker sejam alteradas.


### 4.2 Raspberry pi Zero

O usuário deve se conectar utilizando o SSH, acessar o diretório dos arquivos (TEC477/TP03/G01) e executar os arquivos usando o makefile presente no repositório. Utilizando o comando `make all`.

### 4.3 Interface remota

Para a execução da interface remota só é necessário realizar o download dos códigos presentes no [repositório](https://github.com/MI-SD-TEC499/Interface-Web-MQTT), e abrir o arquivo `index.html` com o seu navegador padrão. 

## 5. Testes de Funcionamento

Para testar o funcionamento do projeto foram realizados alguns testes para as funcionalidades presentes.

### 5.1 Interface remota

Na interface remota foram testadas de duas formas diferentes, com o broker presente no laboratório LEDS e com um broker disponível gratuitamente online. 
Para executar o teste com o broker online deve substituir a linha do broker do laboratório pela apresentada abaixo: 

```js
  const pahoHandler = new PahoHandler('test.mosquitto.org', 8080, "clientjs");
```

Nos dois casos a exibição será a mesma, porém para testes de funcionalidade é um pouco diferente. Utilizando o broker do laboratório, as informações serão recebidas através da conexão com o nodeMCU, definindo o subscribe como `pbl3/envia` e o topic como `pbl3/recebe`, com o send mensage com um valor inteiro para definir o tempo de requisição. Para o teste com o broker remoto informado acima é necessário colocar o tópico de subscribe e o topic com os mesmo valores por exemplo: `pbl3/teste` e o valor de send mensage com os valores para a temperatura, umidade e potênciomentro, separados por ";".

![image](https://user-images.githubusercontent.com/38465439/206821051-e6896cce-d9bb-4332-8a57-f4deedaf1576.png)

Nos dois processos o resultado de exibição será o mesmo, o gráfico irá exibir os valores de acordo com a variação que acontecer na variáveis de temperatura, umidade e potênciomentro.

![image](https://user-images.githubusercontent.com/38465439/206821148-187f3705-790c-4624-b328-65a86795e24c.png)

## 6. Referências

* [Wiring pi](http://wiringpi.com/)
* [PubSubClient](https://github.com/knolleary/pubsubclient)
* [ChartJS](https://www.chartjs.org/docs/latest/)
* [Eclipse paho](https://www.eclipse.org/paho/index.php?page=clients/js/index.php)




