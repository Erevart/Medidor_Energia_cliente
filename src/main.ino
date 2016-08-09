  /*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>

extern "C" {
#include "user_interface.h"
#include <espconn.h>
#include <mem.h>
}

#include <ESP8266WebServer.h>

#include "EEPROM.h"
#include "def.h"
#include "GPIO.h"
#include "Memoria.h"
#include "ComWifi.h"
#include "ESPWifi.h"

void setup() {

   delay(2000);

    EEPROM.begin((MAX_USUARIOS*6 + 2)*sizeof(uint8_t));

  /**********************************/
  /*   Definicion Puerto I/O        */
  /**********************************/
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(0, INPUT);

  /******************************/
  /*   Configuración UART       */
  /******************************/
    Serial.begin(115200);
    delay(1000);

	/******************************/
	/*   Configuración Wifi       */
	/******************************/
    comprobar_eeprom();

    configWifi();

  /*******************/
  /* Interrupciones  */
  /*******************/
    WiFi.onEvent(isrWifi,WIFI_EVENT_SOFTAPMODE_STACONNECTED);
    WiFi.onEvent(isrWifi,WIFI_EVENT_SOFTAPMODE_STADISCONNECTED);
    delay(1000);
    attachInterrupt(0,isrsinc,FALLING);

    delay(100);
}

void loop() {
 // Se actualiza la variable tiempo, para conocer el tiempo transcurrido
 currentMillis = millis();

  if (currentMillis - previousMillis >= MAX_PERIODO) {
 //if (currentMillis == -1){
    // Se guarda en el último instante de tiempo en el,
    // que se ejecuta el contenido del bucle principal.
    previousMillis = currentMillis;

    /**************************************
      Frecuencia de Refresco: 50 Hz
    *************************************/
    if (timecounter % loop1 == 0){

   } else if ((currentMillis - loop1_previousTime) >= 20){
       // Se guarda en el último instante de tiempo en el,
       // que se ejecuta el contenido el loop1.
       loop1_previousTime = currentMillis;
   }


    /**************************************
      Frecuencia de Refresco: 25 Hz
    *************************************/
    if (timecounter % loop2 == 0){

    }
    else if ( (currentMillis - loop2_previousTime) >= 50){
      // Se guarda en el último instante de tiempo en el,
      // que se ejecuta el contenido el loop2.
      loop2_previousTime = currentMillis;

    }

    /**************************************
      Frecuencia de Refresco:  1 Hz
    *************************************/
    if (timecounter % loop3 == 0){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

      // Se establece comunicación con el servidor.
      if (red_usuarios.numusu && usuario_conectado != NULL )
        if (usuario_conectado->estado){

        //  comunicacion_servidor();
        }
    }

    /**************************************
      Frecuencia de Refresco:  1/7 Hz
    *************************************/
    if (timecounter % loop4 == 0){

      if (modo_sinc == SINCRONIZACION){
        Serial.println("Insertar");
        // Se actualiza los usuarios conectados a la red.
        actualizar_red(&red_usuarios);
        // Se actualiza el modo de sincronizacion
        modo_sinc = ESPERA_USUARIOS;
      } else if (modo_sinc == ACTUALIZACION){
        Serial.println("Actualizar");
        actualizacion_estado_usuarios(&red_usuarios);
        // Se actualiza el modo de sincronizacion
        modo_sinc = ESPERA_BOTON;
      }

    }

    /**************************************
      Frecuencia de Refresco:  1/60 Hz
    *************************************/
    if (timecounter % loop4 == 0){

      if (usuario_conectado == NULL){
        usuario_conectado = red_usuarios.usuarios;
        Serial.println("Servidor 0");
      }
      else if (usuario_conectado->siguiente == NULL){
        usuario_conectado = red_usuarios.usuarios;
        Serial.println("Servidor 1");
      } else if ( usuario_conectado->siguiente != NULL){
        usuario_conectado = usuario_conectado->siguiente;
        Serial.println("Servidor 2");
      }

      if (red_usuarios.numusu && usuario_conectado != NULL )
        if (usuario_conectado->estado){
          Serial.print("Cambio de servidor: ");
          Serial.println(usuario_conectado->ipdir,HEX);
      //    conexion_servidor(usuario_conectado->ipdir,false);
          tcp_comunication(usuario_conectado->ipdir);
        }
    }

    // La variable timecounter debe reiniciarse cuando se alcance
    // el numero de ciclos de las acciones de menor prioridad.
    if (timecounter == 6000)
      timecounter = 0;
    else
      timecounter++;

  }

}
