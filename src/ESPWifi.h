/*

  ESPWifi.c - Funciones de comunicacion para la comunicación TCP entre dispositivos ESP8266 - ESP8266.

 */


#include <Arduino.h>

 /******************************************************************************
  * Función : cmp_bssid
  * @brief  : Realiza la comparación entre dos bssid.
  * @param  : mac1 - puntero al bssid del dispositivo 1
  * @param  : mac2 - puntero al bssid del dispositivo 2
  * @return : true - El bssid de ambos dispositivos son iguales.
  * @return : false - El bssid de cada dispositivos es diferentes.
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "CMPBSSID".
  *******************************************************************************/
bool cmp_bssid(char *mac1, char *mac2){

   for (int i = 0; i<6; i++){
 #ifdef _DEBUG_BSSID
     debug.print("[CMPBSSID] MAC1: ");
     debug.println(mac1[i],HEX);
     debug.print("[CMPBSSID] MAC2: ");
     debug.println(mac2[i],HEX);
 #endif
     if (mac1[i] != mac2[i])
       return false;
   }

   return true;
 }


 /******************************************************************************
  * Función : ins_usu
  * @brief  : Actualiza el número y estado de usuarios en la lista de dispositivos registrados.
  * @param  : red - puntero a la lista de dispositivos registrados en la red.
  * @param  : nuevo_usu - puntero al dispositivo conectado a la red.
  * @return : >0 - El usuario ha sido registrado y actualizado en la lista de dipositivos correctamente.
  * @return : <0 - El usuario no es registrado, debio a que no se cofirmado correctamente la conexión.
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "INUSU".
  *******************************************************************************/
int8_t ins_usu (lista_usuarios *red, station_info *nuevo_usu){

  struct infousu *nuevo_usuario;
  struct infousu **usuario_actual = &red->usuarios;

  // Se solicita especio para una nueva estructura de datos infousu.
  // En el supuesto de no haber espacio se devuelve -1, cualquiero otro caso 0.
  if ((nuevo_usuario = (infousu *) os_malloc (sizeof (infousu))) == NULL)
    return -1;
  // Se solicita espacio para la variable ip.
  if ((nuevo_usuario->ipdir  = (uint32_t) os_malloc (sizeof (uint32_t))) == NULL)
    return -1;
  // Se solicita espacio para la variable bssid de la estructura
  if ((nuevo_usuario->bssid  = (uint8_t *) os_malloc (6 * sizeof (uint8_t))) == NULL)
      return -1;

  // Se actualiza la ip del nuevo usuario, el puntero hacia el siguiente.
  nuevo_usuario->ipdir = nuevo_usu->ip.addr;
  strcpy(reinterpret_cast<char*>(nuevo_usuario->bssid), reinterpret_cast<char*>(nuevo_usu->bssid));
  nuevo_usuario->siguiente = NULL;

  if (red->numusu == 0){
    if (confirmar_conexion(nuevo_usuario->ipdir)){
      // Se confirma su estado de activado.
      nuevo_usuario->estado = true;
      // Se incremente el número de usuarios registrados.
      red->numusu++;
      // Se añade el nuevo usuario a la lista de usuarios.
      *(usuario_actual) = nuevo_usuario;
      #ifdef _DEBUG_WIFI
        debug.print("[INUSU] Direccion ip cliente ");
        debug.print(0);
        debug.print(": ");
        debug.println((*usuario_actual)->ipdir);
        debug.print("[INUSU] Direccion bssid: ");
        for (int i = 0; i<6; i++){
          debug.print((*usuario_actual)->bssid[i],HEX);
          debug.print(":");
        }
        debug.println();
      #endif
    }
  }
  else
    // Se localiza el extemo de la cadena para insertar un nuevo usuario.
    for (int i = 0; i < red->numusu; i++){
      // Se comprueba si el usuario actual no enlaza con uno siguiente. Si es el
      // último eslabon se añade el nuero usario, de caso contrario se sigue
      // avanzando en la cadena.
      if (cmp_bssid(reinterpret_cast<char*>((*usuario_actual)->bssid), reinterpret_cast<char*>(nuevo_usuario->bssid))){
      #ifdef _DEBUG_WIFI
        debug.println("[INUSU] Usuario ya registrado");
      #endif
        if ((*usuario_actual)->ipdir != nuevo_usuario->ipdir){
          (*usuario_actual)->ipdir = nuevo_usuario->ipdir;
        }

        if (!(*usuario_actual)->estado)
          if (confirmar_conexion(nuevo_usuario->ipdir)){
            (*usuario_actual)->estado = true;
            #ifdef _DEBUG_COMUNICACION
                debug.println("[INUSU] Usuario activado");
            #endif
          }
          else{
            (*usuario_actual)->estado = false;
            #ifdef _DEBUG_COMUNICACION
                debug.println("[INUSU] Usuario NO activado");
            #endif
          }

        os_free(nuevo_usuario);
        return 0;
      }
      if ((*usuario_actual)->siguiente == NULL){
        // Se confirma el registro del usuario
        if (confirmar_conexion(nuevo_usuario->ipdir)){
          // Se confirma su estado de activado.
          nuevo_usuario->estado = true;
          // Se incremente el número de usuarios registrados.
          red->numusu++;
          // Se añade el nuevo usuario a la lista de usuarios.
          (*usuario_actual)->siguiente = nuevo_usuario;
        #ifdef _DEBUG_WIFI
            debug.print("[INUSU] Direccion ip cliente ");
            debug.print(i);
            debug.print(": ");
            debug.println((*usuario_actual)->ipdir);
            debug.print("[INUSU] Direccion bssid: ");
            for (int i = 0; i<6; i++){
              debug.print((*usuario_actual)->bssid[i],HEX);
              debug.print(":");
            }
            debug.println();
          #endif
        }
        break;
      }

      #ifdef _DEBUG_WIFI
        debug.print("[INUSU] Direccion ip cliente ");
        debug.print(i);
        debug.print(": ");
        debug.println((*usuario_actual)->ipdir);
        debug.print("[INUSU] Direccion bssid: ");
        for (int i = 0; i<6; i++){
          debug.print((*usuario_actual)->bssid[i],HEX);
          debug.print(":");
        }
        debug.println();
      #endif

      // Se modifica el puntero al siguiente usuario de la lista de usuarios registrados.
      usuario_actual = &(*usuario_actual)->siguiente;
    }

  return 0;
 }

 /******************************************************************************
  * Función : actualizacion_estado_usuarios
  * @brief  : Actualiza el estado de los usuarios ya registrados.
  * @param  : red - puntero a la lista de dispositivos registrados en la red.
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "AEU".
  *******************************************************************************/
void actualizacion_estado_usuarios(lista_usuarios *red){

  // Si el número de usuarios registrados o conectados es cero no se realizada nada.
  if (red->numusu == 0 || red->numconex == 0)
    return;

  ets_intr_lock();

  struct station_info *infored;
  struct infousu **usuario_actual = &red->usuarios; // Variable para manejar la información de los diferntes usuarios
  struct station_info *infored_actual;              // Variable para manejar la información de la red de usuarios.

  #ifdef _DEBUG_WIFI
   debug.println("[AEU] INICIO, actualizacion lista de usuarios");
  #endif

  // Se identifica el número de usuarios conectados.
  infored = wifi_softap_get_station_info();

  for (int i = 0; i < red->numusu; i++){
    infored_actual = infored;
    #ifdef _DEBUG_WIFI
        Serial.println("[AEU] Seleciono usuario registrado");
    #endif
     while (infored_actual != NULL) {
      #ifdef _DEBUG_WIFI
            Serial.println("[AEU] Seleciono usuario nuevo");
      #endif
      // Si el bssid es diferente se indica que el usuario no es detectado, y por lo tanto no esta conectado.
      if (!cmp_bssid(reinterpret_cast<char*>((*usuario_actual)->bssid), reinterpret_cast<char*>(infored_actual->bssid))){
        // Al no ser encontrado el dispositivo registrado en la lista de usuarios conectados se indica que se encuentra
        // desconectado.
        (*usuario_actual)->estado = false;
        #ifdef _DEBUG_WIFI
          debug.println("[AEU] Usuario con diferente mac");
          debug.print("[AEU] Direccion bssid nuevo usuario: ");
          for (int i = 0; i<6; i++){
            debug.print(infored_actual->bssid[i],HEX);
            debug.print(":");
          }
          debug.println();
          debug.print("[AEU] Direccion bssid usuario registrado: ");
          for (int i = 0; i<6; i++){
            debug.print((*usuario_actual)->bssid[i],HEX);
            debug.print(":");
          }
          debug.println();
          debug.println("[AEU] Comparacion mac");
          for (int i = 0; i<6; i++){
            if (infored_actual->bssid[i] == (*usuario_actual)->bssid[i]){
              debug.print("[AEU] Igual: ");
              debug.println(infored_actual->bssid[i]);
            }else {
              debug.print("[AEU] Diferente: ");
              debug.println(infored_actual->bssid[i]);
            }
          }
        debug.println();
        #endif
      }
      // El bssid del usuario conectado y registrado coinciden. Por lo tanto se encuentra activo (conectado).
      else {
        #ifdef _DEBUG_WIFI
          debug.println("[AEU] Coincide mac");
        #endif
        // Se comprueba si presenta un dirección IP diferente a la registrada
        if ((*usuario_actual)->ipdir != infored_actual->ip.addr){
            (*usuario_actual)->ipdir = infored_actual->ip.addr;
        }
        // En el caso de no encontrarse activado, se realiza el proceso de confirmación
        // de la conexión.
        if (!(*usuario_actual)->estado)
          // Si el proceso de confirmación de la conexión no es realizado con éxito,
          // se indica que el dispositivo se encuenta desactivado.
          if (confirmar_conexion((*usuario_actual)->ipdir)){
              (*usuario_actual)->estado = true;
              #ifdef _DEBUG_COMUNICACION
                  debug.println("Usuario activado");
              #endif
          } else{
              (*usuario_actual)->estado = false;
              #ifdef _DEBUG_COMUNICACION
                  debug.println("Usuario NO activado");
              #endif
          }
        #ifdef _DEBUG_WIFI
          debug.print("[AEU] Direccion ip cliente ");
          debug.print(i);
          debug.print(": ");
          debug.println((*usuario_actual)->ipdir);
          debug.print("[AEU] Direccion bssid: ");
          for (int j = 0; j<6; j++){
            debug.print((*usuario_actual)->bssid[j],HEX);
            debug.print(":");
          }
          debug.println();
        #endif
        break;
      }
      // Se modifica el puntero al siguiente usuario de la lista de usuarios conectados.
      infored_actual = STAILQ_NEXT(infored_actual, next);
    }
      // Se modifica el puntero al siguiente usuario de la lista de usuarios registrados.
      usuario_actual = &(*usuario_actual)->siguiente;
  }
  wifi_softap_free_station_info();

  #ifdef _DEBUG_WIFI
   debug.println("[AEU] FIN, actualizacion lista de usuarios");
  #endif

  ets_intr_unlock();
 }

 /******************************************************************************
  * Función : borrar_usuarios
  * @brief  : Actualiza el número de usuarios y su estado registrados en la red.
  * @param  : red - puntero a la lista de dispositivos registrados en la red.
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "BORRUSU".
  *******************************************************************************/
void borrar_usuarios(lista_usuarios *red){

  infousu *sup_usuario;

  #ifdef _DEBUG_WIFI
     debug.println("[BORRUSU] Se borran todos los usuarios.");
  #endif

  // Se recorre la lista de usuarios
  while (red->usuarios != NULL){
    sup_usuario = red->usuarios;
    // Se cambia el puntero al siguiente usuario.
    red->usuarios = red->usuarios->siguiente;
    red->numusu--;
    #ifdef _DEBUG_WIFI
       debug.print("[BORRUSU] Numero de usuarios actuales: ");
       debug.println(red->numusu);
    #endif
    // Se borra el usuario selecionado.
    os_free(sup_usuario);
  }

  // Primer dirección en la memoria indica si hay datos guardados.
  // Se indica que no hay datos registrados.
  nvrWrite_u8(0,0);

  // Se pone a cero el numero de usuarios registrados en memoria.
  nvrWrite_u8(0,1);

  // Se actualiza la memoria
  EEPROM.commit();

  #ifdef _DEBUG_WIFI
     debug.println("[BORRUSU] Todos los usuarios borrados.");
  #endif

  return;
}

/******************************************************************************
 * Función : actualizar_red
 * @brief  : Actualiza el número de usuarios y su estado registrados en la red.
 * @param  : red - puntero a la lista de dispositivos registrados en la red.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "ACTRED".
 *******************************************************************************/
void actualizar_red(lista_usuarios *red){

  ets_intr_lock();

  struct station_info *infored;

  // Se obtiene la información de usuarios conectados.
  infored = wifi_softap_get_station_info();
  #ifdef _DEBUG_WIFI
    debug.println("[ACTRED] Se va a actualizar la lista de usuarios");
  #endif

  while (infored != NULL) {

  #ifdef _DEBUG_WIFI
      debug.println("[ACTRED] Se crea un nuevo usuario.");
      int8_t i = ins_usu(&red_usuarios,infored);
      if (i == -1)
        debug.println("[ACTRED] No se ha obtenido espacio en memoria");
      else if (i == 0)
        debug.println("[ACTRED] Se ha obtenido espacio en memoria");
  #else
      // Comprueba si el usuario existe en la red, sino existiera lo añade y indica su estado.
      ins_usu(&red_usuarios,infored);
  #endif

    // Se modifica el puntero al siguiente usuario.
    infored = STAILQ_NEXT(infored, next);
  }
  wifi_softap_free_station_info();
  #ifdef _DEBUG_WIFI
    debug.println("[ACTRED] Se guardan los datos de los usuarios en memoria.");
  #endif

  // Se guarda los usuarios registrados en memoria.
  guardar_red(&red_usuarios);

  ets_intr_unlock();
}


/******************************************************************************
 * Función : timersoft
 * @brief  : Interrupción software que activa el estado de sincronización o de borrado de los
              dipositivos registrados en memoria.
 * @param  : pArg - puntero que indica el timer que ha saltado la interrupción.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "ISRTS".
 *******************************************************************************/
void timersoft(void *pArg){

  ets_intr_lock();

 // Se comprueba el origen de la interrupción.
 // La interrupción es debida al pulsador (no activado) y timer de sincronización.
 if (digitalRead(GPIO_SINC) == HIGH && pArg == timersinc){

   struct softap_config *config = (struct softap_config *)os_malloc(sizeof(struct
   softap_config));

   modo_sinc = ESPERA_BOTON;

   // Se lee la configuración actual de la red.
   wifi_softap_dhcps_stop();
   wifi_softap_get_config(config);
   config->ssid_hidden = 1;

   // Las nuevas modificaciones son grabadas en la memoria flash.
   wifi_softap_set_config_current(config);
   wifi_softap_dhcps_start();
   os_free(config);

   // Se desactiva el timer de reset y de sincronización.
   os_timer_disarm(timersinc);
   os_timer_disarm(timerreset);

   os_free(timersinc);
   os_free(timerreset);

#ifdef _DEBUG_WIFI
   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
   debug.println("[ISRTS] Timer deshabilitado. SSID ocultado.");
#endif

  }
  // La interrupción es debida al pulsador (activado) y timer de reset de memoria.
  else if (digitalRead(GPIO_SINC) == LOW && pArg == timerreset) {

    // Se activa el timer de reset
    os_timer_disarm(timerreset);

    os_free(timerreset);

    // Se borran los datos de los usuarios registrados
    borrar_usuarios(&red_usuarios);
#ifdef _DEBUG_WIFI
    debug.println("[ISRTS] Se borran los datos de usuarios");
#endif
  }

  ets_intr_unlock();

 }

 /******************************************************************************
  * Función : isrsinc
  * @brief  : Interrupción del pulsador de sincronización y reset, permite
              identificar cuando se solicita activar el modo de sincronización.
  * @param  : none
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "ISRS".
  *******************************************************************************/
void isrsinc(){

  ets_intr_lock();

  // Se ha pulsado el pulsador se sincronización, se busca el nuevo usuario he
  // indica que se ha conectado correctamente a la red.
  if(digitalRead(GPIO_SINC) == LOW){
    timersinc = (os_timer_t*)os_malloc(sizeof(os_timer_t));
    timerreset = (os_timer_t*)os_malloc(sizeof(os_timer_t));
    struct softap_config *config = (struct softap_config *)os_malloc(sizeof(struct
    softap_config));

    // Se activa el modo de sincronización
    modo_sinc = ESPERA_USUARIOS;

    // Se lee la configuración actual de la red.
    wifi_softap_dhcps_stop();
    wifi_softap_get_config(config);
    config->ssid_hidden = 0;

    // Las nuevas modificaciones son grabadas en la memoria flash.
    wifi_softap_set_config_current(config);
    wifi_softap_dhcps_start();
    os_free(config);

    // Se realiza una previa desactivación del timer.
    os_timer_disarm(timersinc);

    // Se activa un timer, si transcurrido un tiempo éste no ha sido desabilitado,
    // el modo se sincronización se deshabilita y desactiva el timer.
    os_timer_setfn(timersinc,timersoft, timersinc);
    os_timer_setfn(timerreset,timersoft, timerreset);

    // Se activa el timer de sincronización
    os_timer_arm(timersinc, TIEMPO_SINCRONIZACION, false);
    os_timer_arm(timerreset, TIEMPO_RESET, false);


    #ifdef _DEBUG_WIFI
      debug.println(" [ISRS] Timer de sincronizacion activado. SSID visible");
    #endif

  }

  ets_intr_unlock();

}

 /******************************************************************************
  * Función : isrWifi
  * @brief  : Interrupción de eventos wifi, permite identificar cuando se ha realizado
  *        la conexión de un nuevo usuario a la red creada.
  * @param  : event - indica el tipo de evento que ha disparado la interrupcion
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "ISRW".
  *******************************************************************************/
void isrWifi (WiFiEvent_t event) {

    ets_intr_lock();

  #ifdef _DEBUG_WIFI
    debug.printf("[ISRW] event: %d\n", event);
  #endif
  // Se actualiza el número de usuarios conectados a la red.
  red_usuarios.numconex = wifi_softap_get_station_num();


  #ifdef _DEBUG_WIFI
    debug.print("[ISRW] Numero de clientes: ");
    debug.println(red_usuarios.numconex);
  #endif

  switch(event) {
     // Usuario conectado
     case WIFI_EVENT_SOFTAPMODE_STACONNECTED:

       if (modo_sinc == ESPERA_USUARIOS){
          modo_sinc = SINCRONIZACION;
       }
       else if (modo_sinc != SINCRONIZACION){
         modo_sinc = ACTUALIZACION;
       }

       #ifdef _DEBUG_WIFI
         debug.println("[ISRW] Usuario conectado.");
       #endif
       break;

     // Usuario desconectado
     case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:

      // Actualizar número de usuarios conectados;
      if (modo_sinc == ESPERA_BOTON)
        modo_sinc = ACTUALIZACION;
       #ifdef _DEBUG_WIFI
        debug.println("[ISRW] Usuario desconectado.");
       #endif
     break;

     default:
      #ifdef _DEBUG_WIFI
        debug.println("[ISRW] Situación no contemplada.");
      #endif
     break;
  }

  ets_intr_unlock();

 }
