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
 * Fichero: protocol.h
 * Versión: 4.1
 * Curso: 2026/2027
 * Descripción: Fichero de encabezado para práctica 1
 * Autor: Juan Carlos Cuevas Martínez
 *
 *******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1:
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/
#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define USER "USER"
#define PASS "PASS"
#define QUIT "QUIT"
#define ECHO "ECHO"
#define ROOM "ROOM" //Comando creado para Tarea 4


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_INIT 0
#define S_USER 1
#define S_PASS 2
#define S_DATA 3
#define S_QUIT 4


//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	60000

// NOMBRE Y CLAVE AUTORIZADOS
// Nota: esto no se debe hacer nunca, es solamente para simplificar el funcionamiento del
// servidor, el cual debería almacenar estos datos en una base de datos de forma segura.
#define TEST_USER		"user" 
#define TEST_PASSWORD	"1234"