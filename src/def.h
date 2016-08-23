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
#define loop1       1         // Loop de 10 ms
#define loop2       2         // Loop de 20 ms
#define loop3       100       // Loop de 1s
#define loop4       700       // Loop de 7s
#define loop5       6000      // Loop de 1m

/* Parametros Red WIFI */
#define PRE_SSID "MCPESP_"
#define CONTRASENA "zxcvmnbv1234"
#define MCPESP_SERVER_PORT 23          // Puerto de conexión a los servidores.
#define MAX_USUARIOS 255               // Número máximo de usuarios conectados a la red.
#define HIDDEN_DEFAULT 1               // El ssid está oculto por defecto.
#define BEACON_INTERVAL 50             // Periodo (ms) de envio de paquetes de confirmación de conexion wifi.

#define TIEMPO_RESET 5000              // Tiempo de espera para el borrado de los datos
#define TIEMPO_SINCRONIZACION 30000    // Tiempo de espera para realizar la sincronización en ms.

/* Estados de sincronización */
#define ESPERA_BOTON 0x00              // Espera
#define ESPERA_USUARIOS 0x01
#define SINCRONIZACION 0x02
#define ACTUALIZACION 0x03


/* Parametros Comunicacion ESP8266 - ESP8266 */
#define MAX_ESPWIFI 15000              // Tiempo de espera antes de considerar que la comunicacion wifi ha sido perdida.
#define USUARIO_REGISTRADO 0xEE        // Comando de registro del dispositivo.
#define WACK 0xCC                      // ACK del comando "USUARIO_REGISTRADO".
#define RTC_MAGIC 0x55aaaa55           // RTC_MAGIC
/* Parametros escritura y lectura en memoria */
#define DATOS_WIFI 0xEF                // Indica que la memoria del dispositivo hay información gurdada respesto a la red de dipositivos.



/* Declaración de estructura */
// Estructura de identificación de los usuarios.
typedef struct infousu{
  struct infousu *siguiente = NULL;   // Puntero al siguiente usuario.
  uint32_t ipdir = -1;                // Ip del usuario.
  uint8_t *bssid;                     // BSSID del usuario
  int8_t estado = -1;                 // Estado del usuario: Conectado/true o Desconectado/false

} infousu;

// Estructura de registro de los usuarios y control de número de usuarios conectados y registrados.
typedef struct lista_usuarios {
  uint8_t numconex = 0;
  uint8_t numusu = 0;
  struct infousu *usuarios = NULL;

} lista_usuarios;

// Estructura de datos RTC
typedef struct {
  uint64 timeAcc;
  uint32 magic;
  uint32 timeBase;
} RTC_TIMER;



/* Prototipo de Funciones */

// comtcp
void tcp_server_sent_cb(void *arg);
void tcp_server_discon_cb(void *arg);
void tcp_server_recon_cb(void *arg, sint8 err);
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length);
void tcp_listen(void *arg);
void configWifi();
bool confirmar_conexion(uint32_t host);
void tcp_comunication(const uint32_t host);

// ESPWifi
bool cmp_bssid(char *mac1, char *mac2);
int8_t ins_usu (lista_usuarios *red, station_info *nuevo_usu);
void actualizacion_estado_usuarios(lista_usuarios *red);
void borrar_usuarios(lista_usuarios *red);
void actualizar_red(lista_usuarios *red);
void timersoft(void *pArg);
void isrsinc();
void isrWifi (WiFiEvent_t event);

// Memoria
void nvrWrite_u8(uint8_t value, unsigned int memaddr);
uint8_t nvrRead_u8(unsigned int memaddr);
void guardar_red(lista_usuarios *red);
void leer_red(lista_usuarios *red );
void comprobacion_usuarios_eeprom();


/* Variables */
struct lista_usuarios red_usuarios;       // Estructura con registro de diposisitivos conectados.
struct infousu *usuario_conectado = NULL; // Estructura con información del dispositivo.
static struct espconn *esp_conn;          // Estructura de comunicación TCP.

os_timer_t *timersinc = NULL;             // Timer software para sincronización.
os_timer_t *timerreset = NULL;            // Timer software para reset.

unsigned long currentMillis = 0;          // Variable que indica el tiempo actual.
unsigned long previousMillis = 0;         // Variable que indica el instante de tiempo en el que se realizo la ultima ejecucion del bucle principal.
unsigned long loop2_previousTime = 0;     // Variable que indica el instante de tiempo en el que se ejecuto el loop2_

/* Aún en pruebas */
uint32_t temp = 0;
uint32_t tension = 0;
uint32_t corriente = 0;
uint32_t frecuencia = 0;
/* Aún en pruebas */
uint32_t timecounter = 0;                 // Variable que indica el numero de iteracciones del bucle principal.


uint8_t modo_sinc = 0x00;                 // Variable que indica es estado en el que se encuentra la máquina de estado,
                                          // para la sincronización de los dispositivos.

bool tcp_establecido = false;             // Indica que la conexión TPC se ha estblecido con el servidor.
bool registro_confirmado = false;         // Variable utilizada para verifizar que la confirmación de
                                          // conexión se ha llevado a cabo correctamente.
bool transmision_finalizada = false;      // Los datos enviados se ha transmitido correctamente.
bool tcp_desconectado = true;             // La comunicación TCP ha sido finalizada.
bool tcp_recibido = false;                // Indica si se ha recibido la información pediente de recibir.
