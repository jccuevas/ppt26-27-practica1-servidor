/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Escuela Politécnia Superior de Linares
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 *******************************************************
 * Fichero: servidor.c
 * Versión: 3.0
 * Notas de la versión: corrección de warnings de la versión 2.2
 * Curso: 2026/2027
 * Descripción: Servidor concurrente de eco sencillo TCP
 *              sobre IPv4/IPv6.
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1:
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <process.h>	// Biblioteca para el empleo de hebras de ejecución
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	// Declarar constantes y funciones de la práctica

#pragma comment(lib, "Ws2_32.lib")

void service(void* socket);

int main(int* argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sockfd, newsockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr* remote_addr = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6  server_in6;
	int address_size = sizeof(struct sockaddr_in6);
	char remoteaddress[128] = "";
	unsigned short remoteport = 0;

	int err;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char option[256];

	//Inicialización de idioma
	setlocale(LC_ALL, "es_ES.UTF8");

	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return(-1);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(-2);
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/
	printf("SERVIDOR> ¿Qué versión de IP desea usar?\r\n\t 6 para IPv6, 4 para IPv4 [por defecto] ");
	gets_s(option, sizeof(ipdest));

	if (strcmp(option, "6") == 0) {
		ipversion = AF_INET6;

	}
	else { //Distinto de 6 se elige la versión 4
		ipversion = AF_INET;
	}


	sockfd = socket(ipversion, SOCK_STREAM, IPPROTO_TCP);

	if (sockfd == INVALID_SOCKET) {
		DWORD error = GetLastError();
		printf("Error %d\r\n", error);
		return (-1);
	}
	else {
		if (ipversion == AF_INET6) {
			memset(&server_in6, 0, sizeof(server_in6));
			server_in6.sin6_family = AF_INET6; // Familia de protocolos IPv6 de Internet
			server_in6.sin6_port = htons(TCP_SERVICE_PORT);// Puerto del servidor
			//inet_pton(ipversion, "::1", &server_in6.sin6_addr);	// Direccion IP del servidor
																	// Se debe cambiar para que conincida con la de la interfaz
																	// del host que se quiera usar
			server_in6.sin6_addr = in6addr_any;//Conexiones de cualquier interfaz y de IPv4 o IPv6
			server_in6.sin6_flowinfo = 0;
			server_in = (struct sockaddr*) & server_in6;
			address_size = sizeof(server_in6);
		}
		else {
			//ipversion == AF_INET
			memset(&server_in4, 0, sizeof(server_in4));
			server_in4.sin_family = AF_INET; // Familia de protocolos IPv4 de Internet
			server_in4.sin_port = htons(TCP_SERVICE_PORT);// Puerto del servidor
			server_in4.sin_addr.s_addr = INADDR_ANY;
			//inet_pton(ipversion, "127.0.0.1", &server_in4.sin_addr.s_addr);//Dirección de loopback
			server_in = (struct sockaddr*) & server_in4;
			address_size = sizeof(server_in4);
		}
	}

	if (bind(sockfd, (struct sockaddr*)server_in, address_size) < 0) {
		DWORD error = GetLastError();
		printf("Error %d\r\n", error);
		return (-2);
	}

	if (listen(sockfd, 5) != 0) {
		DWORD error = GetLastError();
		printf("Error %d\r\n", error);
		return (-3);
	}

	// El servidor espera conexiones en un bucle infinito. 
	// Lo debe parar el administrador
	while (1) {
		printf("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n");
		remote_addr = malloc(address_size);

		newsockfd = accept(sockfd, (struct sockaddr*)remote_addr, &address_size);
		if (newsockfd == INVALID_SOCKET) {
			DWORD error = GetLastError();
			printf("Error %d\r\n", error);
		}
		else {
			//Se comprueba si la dirección es IPv6 para mostrarla
			if (ipversion == AF_INET6) {
				struct sockaddr_in6* temp = (struct sockaddr_in6*)remote_addr;
				if (temp != NULL) {
					inet_ntop(AF_INET6, &(temp->sin6_addr), remoteaddress, sizeof(remoteaddress));
					remoteport = ntohs(temp->sin6_port);
				}
				else {
					printf("SERVIDOR> ERROR: No se pudo obtener la dirección remota\r\n");
					return(-4);
				}
			}
			else {
				//Si no es IPv6 se supone IPv4
				struct sockaddr_in* temp = (struct sockaddr_in*)remote_addr;
				if (temp != NULL) {
					inet_ntop(AF_INET, &(temp->sin_addr), remoteaddress, sizeof(remoteaddress));
					remoteport = ntohs(temp->sin_port);
				}
				else {
					printf("SERVIDOR> ERROR: No se pudo obtener la dirección remota\r\n");
					return(-4);
				}
			}

			printf("SERVIDOR> CLIENTE CONECTADO DESDE %s:%u\r\n", remoteaddress, remoteport);

			_beginthread(service, 0, (void*)&newsockfd);//Se le pasa a la hebra de atención el nuevo socket
		}

	}

	printf("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
}

/*
  Función que incorpora el código de atención a cada cliente que se ejecuta en una hebra
  En esta función es donde se modela el protocolo de aplicación del servicio que ofrece 
  el servidor.
  Parámetros:
  - void* socket: puntero al socket con el cliente.
*/
void service(void* socket) {
	SOCKET* newsockfd;
	char buffer_out[1024], buffer_in[1024];
	char command[5]; //Máximo 4 caracteres de comando más 0x00 de finalización de cadena
	char user[9]; // Máximo 8 caracteres de usuario más 0x00 de finalización de cadena
	char password[9]; // Máximo 8 caracteres de password más 0x00 de finalización de cadena
	int close_connection = 0;
	int received = 0, sent = 0;
	int status = 0;
	SOCKADDR remote;
	struct sockaddr_in* remote4;
	struct sockaddr_in6* remote6;
	int remote_len = sizeof(remote);
	int port = 0;
	char remote_addr[1024];


	if (socket == NULL) {
		//No se ha recibido un socket 
		return;
	}

	newsockfd = (SOCKET*)socket;

	//Se obtiene la dirección del otro extremo para mostrarla en la salida estándar
	getpeername(*newsockfd, &remote, &remote_len);

	//Se comprueba si es IPv4 o IPv6
	if (remote.sa_family == AF_INET) {
		remote4 = (struct sockaddr_in*) & remote;
		inet_ntop(AF_INET, &(remote4->sin_addr), remote_addr, sizeof(remote_addr));
		port = ntohs(remote4->sin_port);
	}
	else {
		remote6 = (struct sockaddr_in6*) & remote;
		inet_ntop(AF_INET6, &(remote6->sin6_addr), remote_addr, sizeof(remote_addr));
		port = ntohs(remote6->sin6_port);
	}

	//Mensaje de Bienvenida
	sprintf_s(buffer_out, sizeof(buffer_out), "%s Bienvenido al servidor Sencillo%s", OK, CRLF);

	sent = send(*newsockfd, buffer_out, (int)strlen(buffer_out), 0);

	//Se reestablece el estado inicial
	status = S_USER;
	close_connection = 0;

	printf("SERVIDOR [CLIENTE EN %s:%d]> Esperando conexion de aplicacion\r\n", remote_addr, port);
	do {
		//Se espera un comando del cliente
		received = recv(*newsockfd, buffer_in, 1023, 0);

		buffer_in[received] = 0x00;// Dado que los datos recibidos se tratan como cadenas
									// se debe introducir el carácter 0x00 para finalizarla
									// ya que es así como se representan las cadenas de caracteres
									// en el lenguaje C

		printf("SERVIDOR RECIBIDO [%s:%d %d bytes]>%s\r\n", remote_addr,port,received, buffer_in);

		//SE analiza el formato de la PDU de aplicación (APDU)
		strncpy_s(command, sizeof(command), buffer_in, 4);
		command[4] = 0x00; // Se finaliza la cadena
		printf("SERVIDOR COMANDO RECIBIDO>%s\r\n", command);

		//Máquina de estados del servidor para seguir el protocolo
		//En función del estado en el que se encuentre habrá unos comandos permitidos
		switch (status) {
		case S_USER:
			if (strcmp(command, USER) == 0) { // si recibido es solicitud de conexion de aplicacion
				strcpy_s(user, (unsigned)sizeof(user), "");//Se limpian las cadenas de usuario y contraseña para evitar problemas con sscanf_s
				strcpy_s(password, (unsigned)sizeof(password), "");

				sscanf_s(buffer_in, "USER %8s\r\n", user, (unsigned)_countof(user));

				// envia OK acepta todos los usuarios hasta que tenga la clave
				sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", OK, CRLF);

				status = S_PASS;
				printf("SERVIDOR> Esperando clave\r\n");
			}
			else
				if (strcmp(command, QUIT) == 0) {
					sprintf_s(buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK, CRLF);
					close_connection = 1;
				}
				else {
					sprintf_s(buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s", ER, CRLF);
				}
			break;

		case S_PASS:
			if (strcmp(command, PASS) == 0) { // si comando recibido es password

				sscanf_s(buffer_in, "PASS %8s\r\n", password, (unsigned)_countof(password));

				if ((strcmp(user, TEST_USER) == 0) && (strcmp(password, TEST_PASSWORD) == 0)) { // si password recibido es correcto
					// envia aceptacion de la conexion de aplicacion con el nombre de usuario
					sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", OK, user, CRLF);
					status = S_DATA;
					printf("SERVIDOR> Esperando comando\r\n");
				}
				else {
					sprintf_s(buffer_out, sizeof(buffer_out), "%s Autenticación errónea%s", ER, CRLF);
					status = S_USER;//Volvemos al estado S_USER para reiniciar la autenticación
				}
			}
			else if (strcmp(command, QUIT) == 0) {
				sprintf_s(buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK, CRLF);
				close_connection = 1;
			}
			else {
				sprintf_s(buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s", ER, CRLF);
			}
			break;

		case S_DATA: 
			buffer_in[received] = 0x00;

			if (strcmp(command, QUIT) == 0) {
				sprintf_s(buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK, CRLF);
				close_connection = 1;
			}
			else if (strcmp(command, ECHO) == 0) {
				char echo[1024];
				sscanf_s(buffer_in, "ECHO %[^\r]\r\n", echo, (unsigned)_countof(echo));// la expresión %[^\r] hace que se busque
																		   // se añada a echo cualquier carácter que no
																		   // sea \r (CR)
				sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", OK, echo, CRLF);
			}
			 
			 if(strcmp(command, ROOM) == 0) {
		// VALIDACIÓN **************************
		// Recomendado: hacer esto en una función aparte
			 //Opcional, asegurar que el comando ROOM tiene solamente cuatro caracteres.
			 // - He recibido: ROOM SP y no, por ejemplo, ROOMO
			 //Opcional, que funcione para mayúsculas y minúsculas en cualquier combinación
			 //El comando ya es ROOM
			 //1º Validamos las longitudes del mensaje
			 //2º Longitud correcta: extraemos parámetros con sscanf_s*/
				 char room[16] = "";
				 unsigned int day = 0;
				 unsigned int month = 0;
				 int read = 0;


				 read = sscanf_s(buffer_in, "ROOM %s %u %u\r\n", room, (unsigned)_countof(room), &day, &month);
				 printf("Parámetros %d", read);
			 //3º ¿El mensaje tiene los parámetros requeridos? (3)
		     //4º Comprobar CODE
		     //5º Comprobar DAY
			 //6º Comprobar MONTH
		//FIN VALIDACIÓN *******************************
			 
			 }
			 
			else {
				sprintf_s(buffer_out, sizeof(buffer_out), "%s Comando incorrecto: %s%s", ER, command, CRLF);
			}
			break;

		default:
			break;

		} // switch

		sent = send(*newsockfd, buffer_out, (int)strlen(buffer_out), 0);

	} while (!close_connection);

	printf("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
	shutdown(*newsockfd, SD_SEND);
	closesocket(*newsockfd);

}