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
#include <ets_sys.h>
#include <osapi.h>
}

#include "EEPROM.h"
#include "def.h"
#include "GPIO.h"
#include "Memoria.h"
#include "comtcp.h"
#include "confwifi.h"
#include "rtctime.h"

void setup() {

  /*****************/
  /* RTC timer     */
  /*****************/
  update_rtc_time(true);


  /*****************/
  /* EEPROM    */
  /*****************/

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
    checkEEPROM();

  /******************************/
  /*   Configuración Wifi       */
  /******************************/
    configWifi();

  /*******************/
  /* Interrupciones  */
  /*******************/
    // Interrupción eventos wifi
    WiFi.onEvent(isrWifi,WIFI_EVENT_SOFTAPMODE_STACONNECTED);
    WiFi.onEvent(isrWifi,WIFI_EVENT_SOFTAPMODE_STADISCONNECTED);
    delay(1000);

    // Interrupción del boton de reset.
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

      get_rtc_time();

      // Se realiza el cambio de usuarios para
      // establecer una comunicacion con ellos.
      if (usuario_conectado == NULL){
        usuario_conectado = red_usuarios.usuarios;
      }
      else if (usuario_conectado->siguiente == NULL){
        usuario_conectado = red_usuarios.usuarios;
      } else if ( usuario_conectado->siguiente != NULL){
        usuario_conectado = usuario_conectado->siguiente;
      }

      if (red_usuarios.numusu && usuario_conectado != NULL )
        if (usuario_conectado->estado){
          tcp_comunication(usuario_conectado->ipdir);
        }

    }

    /**************************************
      Frecuencia de Refresco:  1/7 Hz
    *************************************/
    if (timecounter % loop4 == 0){

      if (modo_sinc == SINCRONIZACION){
        // Se actualiza los usuarios conectados a la red.
        check_red(&red_usuarios);
        // Se actualiza el modo de sincronizacion
        modo_sinc = ESPERA_USUARIOS;
      } else if (modo_sinc == ACTUALIZACION){
        sync_users(&red_usuarios);
        // Se actualiza el modo de sincronizacion
        modo_sinc = ESPERA_BOTON;
      }

    }

    /**************************************
      Frecuencia de Refresco:  1/60 Hz
    *************************************/
    if (timecounter % loop4 == 0){

    }

    // La variable timecounter debe reiniciarse cuando se alcance
    // el numero de ciclos de las acciones de menor prioridad.
    if (timecounter == 6000)
      timecounter = 0;
    else
      timecounter++;

  }

}
