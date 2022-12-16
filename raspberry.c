#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <wiringPi.h>          
#include <lcd.h>         

#define SERIAL_PORT_PATH    "/dev/ttyS0"
#define LCD_RS  6               //Register select pin
#define LCD_E   31              //Enable Pin
#define LCD_D4  26              //Data pin 4
#define LCD_D5  27              //Data pin 5
#define LCD_D6  28              //Data pin 6
#define LCD_D7  29              //Data pin 7
#define BUTTON2 24              //Button 2
#define BUTTON3 25	        //Button 3

struct termios g_tty;
int g_fd;
uint8_t l_buff[256];
uint32_t l_len_buff = 256;
int lcd,buttonLoop=0,menu,tempoAux=10,button2Loop=0;

/*
* Função para exibição de dados no displau LCD
*/

void send_to_node(uint8_t comando);
void receive_from_node(); 

void scanButton (int button){

  
  if(button==24){
     if (digitalRead (button) == HIGH){ // Low is pushed
        return ;
     }
     while (digitalRead (button) == LOW){       // Wait for release
        delay (10) ;
	tempoAux++;
	lcdClear(lcd);
          lcdPosition(lcd,0,0);
          send_to_node(0x09); //leitura do resultado obtido pela comunicação com a NodeMCU
          sleep(1);
          receive_from_node();
          sleep(1);
        // delay(1000);
      }
      
  printf("Botão pressionado\n");    
  }

  if(button==25){
     if (digitalRead (button) == HIGH){ // Low is pushed
        return ;
     }
     while (digitalRead (button) == LOW){       // Wait for release
        delay (10) ;
        // delay(1000);
        lcdClear(lcd);
      }
      buttonLoop=1;
      menu++;
   }


}


void print_display(uint8_t *resposta){
    int code = resposta[0]; //variável auxiliar
    printf("Valor recebido :%d \n", resposta[1]-48); 
    switch(code){
	case 0x00: //caso a comunicação com a NodeMCU esteja funcionando corretamente
	        lcdPrintf(lcd,"NODEMCU: ON");    
        break;
	case 0x09: //caso a comunicação com a NodeMCU não esteja funcionando corretamente
        	lcdPrintf(lcd,"NODEMCU: OFF");    
        break;
        case 0x01: //exibição do valor obtido do potênciomento
            //resposta[0] = 0;
        	lcdPrintf(lcd,"Potenciometro:%d", resposta[1]);
        break;
        case 0x02: //exibição do valor obtido do sensor de temperatura
        	//resposta[0] = 0;
		if(resposta[1] == 49){
                lcdPrintf(lcd,"Temperatura: ON");}
		else{
		   lcdPrintf(lcd, "Temperatura: OFF");
		}
        break;
        case 0x03: //exibição do valor obtido do sensor de umidade
        	//resposta[0] = 0;
		if(resposta[1] == 49){
            lcdPrintf(lcd,"Umidade: OFF");}
		else{
		lcdPrintf(lcd, "Umidade: ON");
	}
        break;
        case 0x04: //exibindo quando a led da NodeMCU fica ativada
        	lcdPrintf(lcd,"LED: OFF");
        break;
        case 0x05: //exibindo quando a led da NodeMCU fica desativada
        	lcdPrintf(lcd,"LED: ON");
	break;
	case 0x07:
        	tempoAux = resposta[1];
		lcdPrintf(lcd,"Tempo: %dS", tempoAux);
	break;
	case 0x08:
		lcdPrintf(lcd,"Conectado ao broker");
	break;
        default: //caso aconteça algum erro
        	lcdPrintf(lcd,"ERRO");        
	    break;
    }
}

/*
* Função que acessa o diretório especificado e retorna o file descriptor da UART
*/
static int file_open_and_get_descriptor(const char *fname) {
    int fd;

    fd = open(fname, O_RDWR | O_NONBLOCK); //Abre arquivo da UART e recebe sua descrição
    if(fd < 0) { //Retorna número negativo caso não encontre o arquivo e informa o erro
        printf("Erro na abertura da UART\n");
    }
    return fd;
}

/*
* Função para realizar o envio do vetor de instrução para a NodeMCU
*/
static int file_write_data(int fd, uint8_t *buff, uint32_t len_buff) {
    return write(fd,buff,len_buff);
}

/*
* Função para realizar a leitura do vetor de resposta da NodeMCU
*/
static int file_read_data(int fd, uint8_t *buff, uint32_t len_buff) {
    return read(fd,buff,len_buff);
}

/*
* Função para fechar o arquivo de leitura da UART
*/
static int file_close(int fd) {
    return close(fd);
}

/*
* Função para iniciar porta serial de comunicação
*/
static void open_serial_port(void) {
    g_fd = file_open_and_get_descriptor(SERIAL_PORT_PATH); //Solicita arquivo da UART 
    if(g_fd < 0) { //Caso tenha algum erro retorna um número negativo
        printf("Problema com a UART...\r\n");
        exit(EXIT_FAILURE);
    }
}

/*
* Função para configurar porta serial de comunicação
*/
static void configure_serial_port(void) {
    if(tcgetattr(g_fd, &g_tty)) { //Obtem os parametros associados ao descritor de arquivo e os armazena na struct termios criado anteriormente
        printf("Problema obtendo a porta...\r\n");
        exit(EXIT_FAILURE);
    }


    // Funções para determinar a baudrate	
    cfsetispeed(&g_tty,B9600);
    cfsetospeed(&g_tty,B9600);
    // Função que faz as configurações finais da uart
    cfmakeraw(&g_tty);

    if(tcsetattr(g_fd,TCSANOW,&g_tty)) {
        printf("Problema obtendo atributos da porta...\r\n");
        exit(EXIT_FAILURE);
    }
}

/*
* Função para encerrar comunicação serial
*/
static void close_serial_port(void) {
    file_close(g_fd);
}

/*
* Função para enviar um vetor para a NodeMCU 
*/
void send_to_node(uint8_t comando){
    uint8_t auxTest[8]; //vetor para auxilio na manipulação
    uint8_t *ponteiro;
	for(int i =0; i<8;i++){
		auxTest[i] = 0;
	}
    ponteiro = &auxTest[0]; //ponteiro apontando para primeiro item da lista, de indice 0
    *ponteiro++ = comando; //no indice 1 coloca o parametro comando    
    
    file_write_data(g_fd,&auxTest[0],sizeof(auxTest)); //função para envio do vetor
}

/*
* Função para receber uma resposta da NodeMCU 
*/
void receive_from_node(){
    uint8_t auxTest[100]; //criação do buffer que recebe os dados
	for(int i =0; i<100; i++){
	auxTest[i] = 0;
	}
    int tamanho = file_read_data(g_fd, (void*)auxTest, 100); //função para receber resposta da nodeMCU

    if(tamanho < 0){
        auxTest[0] = 0x09;
        print_display(auxTest);
        return; file_write_data(g_fd,&auxTest[0],sizeof(auxTest)); //função para envio do vetor

    }else{
        auxTest[tamanho] = '\0';
    }
	printf("%c", auxTest[0]);
    print_display(auxTest);
}

int main(void) {
    printf("Sensores NodeMCU\r\n");

    open_serial_port(); //abertura da porta serial para comunicação

    configure_serial_port(); //configuração da porta serial
    
    //variáveis para auxilio do menu
    int aux=0; 
   // int menu=0; 
    
    char *ptr; //ponteiro de char para ser utilizado na função strtol
    long value; //valor do resultado da transformação de vetor para long
    uint8_t comando;  
    wiringPiSetup(); 	
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0); //pinando o display	 



while(aux==0){

     while(buttonLoop==0){
  	scanButton(BUTTON3);
	scanButton(BUTTON2);
     }

    printf("menu :%d\n" ,menu);
    switch(menu){
    	//Temperatura
    	case 1:  
	  lcdPosition(lcd,0,0);
    	  comando = 0x02;
    	  send_to_node(comando); //leitura do resultado obtido pela comunicação com a NodeMCU
	  sleep(1);
	  receive_from_node();
	  sleep(1);
	break;
	
	//Umidade
	case 2:
	  lcdPosition(lcd,0,0);
    	  comando = 0x01;
    	  send_to_node(comando); //leitura do resultado obtido pela comunicação com a NodeMCU
	  sleep(1);
	  receive_from_node();
	  sleep(1);  
	break;
    
        //Potenciometro
    	case 3:
	  lcdPosition(lcd,0,0);
    	  comando = 0x04;
    	  send_to_node(comando); //leitura do resultado obtido pela comunicação com a NodeMCU
	  sleep(1);
	  receive_from_node();
	  sleep(1);    
	break;
	default:
	  lcdClear(lcd);
	  lcdPosition(lcd,0,0);
    	  comando = 0x08;
    	  send_to_node(comando); //leitura do resultado obtido pela comunicação com a NodeMCU
	  sleep(1);
	  receive_from_node();
	  sleep(1);
	  menu=0;
	break;
    
    }
    
    buttonLoop=0;	   

}
    
    close_serial_port(); //encerrando a porta serial de comunicação

    return 0; //encerramento do programa
}


