/*

 ESPWifi.h - WiFi parameter for MCP-ESP comunication

 */


#include <Arduino.h>


/* Declaración de funciones */

/* Declaración de variables */

/*

 ESPWifi.c - WiFi parameter for MCP-ESP

 */

 /*
  *
  */

 int8_t cmp_bssid(char *mac1, char *mac2){

   for (int i = 0; i<6; i++){
 #ifdef _DEBUG_BSSID
     debug.print("MAC1: ");
     debug.println(mac1[i],HEX);
     debug.print("MAC2: ");
     debug.println(mac2[i],HEX);
 #endif
     if (mac1[i] != mac2[i])
       return 0;
   }

   return 1;
 }


/*
 *
*/
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
      debug.print("Direccion ip cliente ");
      debug.print(0);
      debug.print(": ");
      debug.println((*usuario_actual)->ipdir);
      debug.print("Direccion bssid: ");
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
      // Se comprueba si el usuario acutal no enlaza con uno siguiente. Si es el
      // último eslabon se añade el nuero usario, de caso contrario se sigue
      // avanzando en la cadena.
      if (cmp_bssid(reinterpret_cast<char*>((*usuario_actual)->bssid), reinterpret_cast<char*>(nuevo_usuario->bssid))){
#ifdef _DEBUG_WIFI
        debug.println("Usuario ya registrado");
#endif
        if ((*usuario_actual)->ipdir != nuevo_usuario->ipdir){
          (*usuario_actual)->ipdir = nuevo_usuario->ipdir;
        }

        if (!(*usuario_actual)->estado)
          if (confirmar_conexion(nuevo_usuario->ipdir)){
            (*usuario_actual)->estado = true;
            #ifdef _DEBUG_COMUNICACION
                debug.println("Usuario activado");
            #endif
          }
          else{
            (*usuario_actual)->estado = false;
            #ifdef _DEBUG_COMUNICACION
                debug.println("Usuario NO activado");
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
          debug.print("Direccion ip cliente ");
          debug.print(i);
          debug.print(": ");
          debug.println((*usuario_actual)->ipdir);
          debug.print("Direccion bssid: ");
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
          debug.print("Direccion ip cliente ");
          debug.print(i);
          debug.print(": ");
          debug.println((*usuario_actual)->ipdir);
          debug.print("Direccion bssid: ");
          for (int i = 0; i<6; i++){
            debug.print((*usuario_actual)->bssid[i],HEX);
            debug.print(":");
          }
        debug.println();
  #endif

      usuario_actual = &(*usuario_actual)->siguiente;
    }

  return 0;
 }

 /*
  *
 */
void actualizacion_estado_usuarios(lista_usuarios *red){

  if (red->numusu == 0 || red->numconex == 0)
    return;

  ets_intr_lock();

  struct station_info *infored;
  struct infousu **usuario_actual = &red->usuarios;        // Variable para manejar la información de los diferntes usuarios
  struct station_info *infored_actual;        // Variable para manejar la información de la red

  #ifdef _DEBUG_WIFI
   debug.println("ACTUALIZACION: INICIO, actualizacion lista de usuarios");
  #endif

  // Se identifica el número de usuarios conectados.
  infored = wifi_softap_get_station_info();

  for (int i = 0; i < red->numusu; i++){
//  while (usuario_actual != NULL){
    infored_actual = infored;
#ifdef _DEBUG_WIFI
    Serial.println("ACTUALIZACION: Seleciono usuario registrado");
#endif
     while (infored_actual != NULL) {
      // Se comprueba ssid
#ifdef _DEBUG_WIFI
      Serial.println("ACTUALIZACION: Seleciono usuario nuevo");
#endif
      // Si el bssid es diferente se indica que el usuario es detectado, y por lo tanto no esta conectado.
      if (!cmp_bssid(reinterpret_cast<char*>((*usuario_actual)->bssid), reinterpret_cast<char*>(infored_actual->bssid))){
        (*usuario_actual)->estado = false;
#ifdef _DEBUG_WIFI
        debug.println("ACTUALIZACION: Usuario con diferente mac");
        debug.print("ACTUALIZACION: Direccion bssid nuevo usuario: ");
        for (int i = 0; i<6; i++){
          debug.print(infored_actual->bssid[i],HEX);
          debug.print(":");
        }
        debug.println();
        debug.print("ACTUALIZACION: Direccion bssid usuario registrado: ");
        for (int i = 0; i<6; i++){
          debug.print((*usuario_actual)->bssid[i],HEX);
          debug.print(":");
        }
        debug.println();
        debug.println("ACTUALIZACION: Comparacion mac");
        for (int i = 0; i<6; i++){
          if (infored_actual->bssid[i] == (*usuario_actual)->bssid[i]){
            debug.print("ACTUALIZACION: Igual: ");
            debug.println(infored_actual->bssid[i]);
          }else {
            debug.print("ACTUALIZACION: Diferente: ");
            debug.println(infored_actual->bssid[i]);
          }
        }
      debug.println();
#endif
      }
      else {
        #ifdef _DEBUG_WIFI
          debug.println("ACTUALIZACION: Coincide mac");
        #endif
        if ((*usuario_actual)->ipdir != infored_actual->ip.addr){
            (*usuario_actual)->ipdir = infored_actual->ip.addr;
        }
        if (!(*usuario_actual)->estado)
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
        debug.print("ACTUALIZACION: Direccion ip cliente ");
        debug.print(i);
        debug.print(": ");
        debug.println((*usuario_actual)->ipdir);
        debug.print("ACTUALIZACION: Direccion bssid: ");
        for (int j = 0; j<6; j++){
          debug.print((*usuario_actual)->bssid[j],HEX);
          debug.print(":");
        }
        debug.println();
  #endif
        break;
      }
      infored_actual = STAILQ_NEXT(infored_actual, next);
    }
      usuario_actual = &(*usuario_actual)->siguiente;
  }
  wifi_softap_free_station_info();

#ifdef _DEBUG_WIFI
 debug.println("ACTUALIZACION:: FIN, actualizacion lista de usuarios");
#endif

  ets_intr_unlock();
 }

 /**
  *
  **/
void borrar_usuarios(lista_usuarios *red){

  infousu *sup_usuario;

#ifdef _DEBUG_WIFI
   debug.println("BORRADO: Se borran todos los usuarios.");
#endif

  while (red->usuarios != NULL){
//  for (int i = 0; i < red->numusu; i++){
      sup_usuario = red->usuarios;
      red->usuarios = red->usuarios->siguiente;
      red->numusu--;
  #ifdef _DEBUG_WIFI
         debug.print("BORRADO: Numero de usuarios actuales: ");
         debug.println(red->numusu);
  #endif
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
         debug.println("BORRADO: Todos borrados.");
#endif

  return;
}

/**
 *
**/

void actualizar_red(lista_usuarios *red){

  ets_intr_lock();

  struct station_info *infored;

  // Se identifica el número de usuarios conectados.
  infored = wifi_softap_get_station_info();
#ifdef _DEBUG_WIFI
  debug.println("SINCRONIZACION: Se va a actualizar la lista de usuarios");
#endif

  while (infored != NULL) {

#ifdef _DEBUG_WIFI
    debug.println("SINCRONIZACION: Se crea un nuevo usuario.");
    int8_t i = ins_usu(&red_usuarios,infored);
    if (i == -1)
      debug.println("SINCRONIZACION: No se ha obtenido espacio en memoria");
    else if (i == 0)
      debug.println("SINCRONIZACION: Se ha obtenido espacio en memoria");
#else
    ins_usu(&red_usuarios,infored);
#endif

    infored = STAILQ_NEXT(infored, next);
  }
  wifi_softap_free_station_info();
#ifdef _DEBUG_WIFI
  debug.println("SINCRONIZACION: Se guardan los datos de los usuarios en memoria.");
#endif
  guardar_red(&red_usuarios);

  ets_intr_unlock();
}

/**
 *
 **/

void timersoft(void *pArg){

  ets_intr_lock();

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

   os_timer_disarm(timersinc);
   os_timer_disarm(timerreset);

   os_free(timersinc);
   os_free(timerreset);

#ifdef _DEBUG_WIFI
   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
   debug.println("Timer deshabilitado. SSID ocultado.");
#endif

  }
  else if (digitalRead(GPIO_SINC) == LOW && pArg == timerreset) {
    // Se activa el timer de sincronización
    os_timer_disarm(timerreset);

    os_free(timerreset);

    // Se borran los datos de los usuarios registrados
    borrar_usuarios(&red_usuarios);
#ifdef _DEBUG_WIFI
    debug.println("Se borran los datos de usuarios");
#endif
  }
  else if (pArg == timeractred){
    actualizar_red(&red_usuarios);

    os_timer_disarm(timeractred);
    os_free(timeractred);
  }

  ets_intr_unlock();

 }

/**
 *  @brief
 **/

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
    debug.println("Timer de sincronizacion activado. SSID visible");
#endif

  }

  ets_intr_unlock();

}

/**
 * @brief Interrupción de eventos wifi, permite identificar cuando se ha realizado
 *        la conexión de un nuevo usuario a la red creada.
 * @param event: indica el tipo de acción produccida en realición a la conexión wifi.
 **/

void isrWifi (WiFiEvent_t event) {

    ets_intr_lock();

#ifdef _DEBUG_WIFI
  debug.printf("[WiFi-event] event: %d\n", event);
#endif
  // Se actualiza el número de usuarios conectados a la red.
  red_usuarios.numconex = wifi_softap_get_station_num();


#ifdef _DEBUG_WIFI
  debug.print("Numero de clientes: ");
  debug.println(red_usuarios.numconex);
#endif

  switch(event) {
     case WIFI_EVENT_SOFTAPMODE_STACONNECTED:

       if (modo_sinc == ESPERA_USUARIOS){
          modo_sinc = SINCRONIZACION;
       }
       else if (modo_sinc != SINCRONIZACION){
         modo_sinc = ACTUALIZACION;
       }

#ifdef _DEBUG_WIFI
         debug.println("Usuario conectado.");
#endif
         break;

     case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:

      // Actualizar número de usuarios conectados;
      if (modo_sinc == ESPERA_BOTON)
        modo_sinc = ACTUALIZACION;


#ifdef _DEBUG_WIFI
         debug.println("Usuario desconectado.");
#endif
         break;

     default:
#ifdef _DEBUG_WIFI
        debug.println("Situación no contemplada.");
#endif
        break;
  }

  ets_intr_unlock();

 }
