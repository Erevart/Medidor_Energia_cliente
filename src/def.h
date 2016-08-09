/*
    Lectura Contador de Energia

*/

/* ============================================================================================================ */
/* ============================================================================================================ */
/*                                        PARAMETROS DE CONFIGURACION
/* ============================================================================================================ */
/* ============================================================================================================ */

//#define MCP39F501     // Comunicacion con Contador de Energia MCP39F501

/* ============================================================================================================ */
/* ============================================================================================================ */
/* ============================================================================================================ */

/* -------------------------------------------------------------------------------------------------------------*/

/* Parametros de Debug */
#define _DEBUG_WIFI   // Muestra por puerto serie información relativa la configuracion wifi.
#define _DEBUG_COMUNICACION_CONEXION
#define _DEBUG_COMUNICACION
#define _DEBUG_COMUNICACION_LIMIT
#define _DEBUG_MEMORIA  // Muuestra información relativa a la escritura y lectura en memoria.
//#define _DEBUG_BSSID
//#define _DEBUG_ERROR  // Muestra los mensajes de error.
//#define _DEBUG_TX     // Muestra la información que transmitiría al MCP.
//#define _DEBUG_RX     // Muestra la información que recibiría del MCP



/* Definicion de Parametros */
#define uart Serial     // Esta definición permite modificar el modulo de comunicación utilizado para
                        // comunicarse con el Contador de Energia.
#define debug Serial    // Esta definición permite modificar el modulo de comunicación utilizado para
                        // mostrar los mensajes de debug.



/* Macros */

/*********************************/
/* Definicion Puertos (ESP82666) */
/********************************/

#define GPIO_SINC  0   // GPIO 0
#define GPIO3  3 // GPIO 3
#define GPIO4  4 // GPIO 4
#define GPIO5  5 // GPIO 5
#define GPIO6  6 // GPIO 6

/***************
/* Constantes */
/**************/

/* Frecuencias de Procesos */
#define MAX_PERIODO     10                                // Periodo tiempo para realizar las lecturas del chips (ms).
// Solo es necesario indicar el periodo de tiempo en segundo con el que debe repetirse la tarea.
// El tiempo se indica aqui ! // (ms* (MAX_PERIODO / 1000000))
#define loop1       1
#define loop2       2
#define loop3       100
#define loop4       700
#define loop5       6000

/* Parametros Red WIFI */
#define PRE_SSID "MCPESP_"
#define CONTRASENA "zxcvmnbv1234"

#define MAX_INTENTOS 1           // Numero de intentos para establecer la comunicacion sino se ha tansmitido correctamente.
#define TIEMPO_SINCRONIZACION 30000 // Tiempo de espera para realizar la sincronizacio en ms.
#define TIEMPO_RESET 5000 // Tiempo de espera para realizar la sincronizacio en ms.
#define TIEMPO_CONEXION 200 // Tiempo de espera para comprobar los usuarios conectados en ms.
#define MAX_ESPWIFI 10000
#define MCPESP_SERVER_PORT 23      // Puerto de conexión a los servidores.
#define MAX_USUARIOS 255
#define HIDDEN_DEFAULT 1  // El ssid está oculto por defecto.
#define BEACON_INTERVAL 50
#define USUARIO_REGISTRADO 0xEE
#define WACK 0xCC
#define DATOS_WIFI 0xEF // Hay datos Wifi
#define ESPERA_BOTON 0x00
#define ESPERA_USUARIOS 0x01
#define SINCRONIZACION 0x02
#define ACTUALIZACION 0x03



/* Declaración de estructura */
typedef struct infousu{
  struct infousu *siguiente = NULL;   // Puntero al siguiente usuario.
  uint32_t ipdir = -1;                // Ip del usuario.
  uint8_t *bssid;                     // BSSID del usuario
  int8_t estado = -1;                 // Estado del usuario: Conectado/true o Desconectado/false

} infousu;

typedef struct lista_usuarios {
  uint8_t numconex = 0;
  uint8_t numusu = 0;
  struct infousu *usuarios = NULL;

} lista_usuarios;


/* Prototipo de Funciones */
void configWifi();
bool conexion_servidor(uint32_t host);
bool confirmar_conexion(uint32_t host, bool check_conex);
void isrsinc();
void _timersinc(void *);
void actualizacion_estado_usuarios();


/* Variables */
uint32_t temp = 0;
uint32_t tension = 0;
uint32_t corriente = 0;
uint32_t frecuencia = 0;


unsigned long currentMillis = 0;    // Variable que indica el tiempo actual.
unsigned long previousMillis = 0;   // Variable que indica el instante de tiempo en el que se realizo la ultima ejecucion del bucle principal.
unsigned long loop2_previousTime = 0; // Variable que indica el instante de tiempo en el que se ejecuto el loop2_
unsigned long loop1_previousTime = 0; // Variable que indica el instante de tiempo en el que se ejecuto el loop1_
uint32_t timecounter = 0;           // Variable que indica el numero de iteracciones del bucle principal.


uint8_t modo_sinc = 0x00;
struct lista_usuarios red_usuarios;
struct infousu *usuario_conectado = NULL;
uint32_t prev_host = 0;
os_timer_t *timersinc = NULL;
os_timer_t *timeractred = NULL;
os_timer_t *timerreset = NULL;
static struct espconn *esp_conn;
bool tcp_establecido = false;
bool registro_confirmado = false;
bool transmision_finalizada = false;
bool tcp_desconectado = true;
bool tcp_recibido = false;
