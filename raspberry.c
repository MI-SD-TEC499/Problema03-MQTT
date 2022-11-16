#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>

const char* username = "user-do-pc"; // usuário do host do broker
const char* password = "senha-do-pc"; // senha do host do broker
const char* ip_address = "10.0.0.101"; // IP do broker

int mqtt_pub(){
	int rc;
	struct mosquitto * mosq;


	mosquitto_lib_init(); // Must be called before any other mosquitto functions.

	mosq = mosquitto_new("mqttTeste", true, NULL); // Create a new mosquitto client instance.

	mosquitto_username_pw_set(mosq, username, password); // Configure username and password for a mosquitto instance. 
	rc = mosquitto_connect(mosq, ip_address, 1883, 60); // Connect to an MQTT broker.

	if(rc != 0){
		printf("Erro ao conectar com o broker: Erro %d\n", rc);
		mosquitto_destroy(mosq); // Use to free memory associated with a mosquitto client instance.
		return -1;
	}
	printf("Sucesso na conexão\n");

	mosquitto_publish(mosq, NULL, "dado", 32, "teste/publish", 0, false); // Publish a message on a given topic.
    // "dado" é o tópico, 32 é o tamanho do payload (bytes) e "teste/publish" é o dado enviado

	mosquitto_disconnect(mosq); // Disconnect from the broker.
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup(); // Call to free resources associated with the library.
	return 0;
}

void on_connect(struct mosquitto *mosq, void *obj, int rc){
	printf("ID: %d\n", * (int *) obj);
	if (rc){
		printf("Erro: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, "dado", 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
	printf("on_mess1\n");
	printf("Topico %s: %s\n", msg->topic, (char *) msg->payload);
	printf("on _mess2\n");
}

int mqtt_sub(){
	int rc, id = 15;

	mosquitto_lib_init();

	struct mosquitto *mosq;

	mosq = mosquitto_new("mqtt/teste", true, &id);
	mosquitto_connect_callback_set(mosq, on_connect); // Set the connect callback. 
	mosquitto_message_callback_set(mosq, on_message); // Set the message callback. 

	mosquitto_username_pw_set(mosq, username, password);
	rc = mosquitto_connect(mosq, ip_address, 1883, 60);

	if(rc){
		printf("Erro ao conectar");
		return -1;
	}

	mosquitto_loop_start(mosq); // This is part of the threaded client interface. 
	mosquitto_loop_stop(mosq, true); // This is part of the threaded client interface. 

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}

void main(){
	mqtt_sub();
	
}
