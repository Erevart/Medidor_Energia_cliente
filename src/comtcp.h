/*

 comtcp.c - Funciones de comunicacion para la comunicación TCP entre dispositivos ESP8266 - ESP8266.

 */


#include <Arduino.h>

/******************************************************************************
 * Función : tcp_server_sent_cb
 * @brief  : Callback cuando la comunicación tcp se finaliza. Indica que la
             comunicación tcp ha finalizado.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_DC_CB".
 *******************************************************************************/
void tcp_server_sent_cb(void *arg)
{
    //Datos enviados correctamente
    transmision_finalizada = true;

    #ifdef _DEBUG_COMUNICACION
      debug.println("[TCP_ST_CB] TRANSMISION REALIZADA CORRECTAMENTE");
    #endif

}

/******************************************************************************
 * Función : tcp_server_discon_cb
 * @brief  : Callback cuando la comunicación tcp se finaliza. Indica que la
             comunicación tcp ha finalizado.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_DC_CB".
 *******************************************************************************/
void tcp_server_discon_cb(void *arg)
{
  // Comunicación cerrada correctamente.
  tcp_desconectado = true;
  tcp_establecido = false;
  transmision_finalizada = true;
  tcp_recibido = false;

  #ifdef _DEBUG_COMUNICACION
    debug.println("[TCP_DC_CB] DESCONEXION REALIZADA CORRECTAMENTE");
  #endif
}

/******************************************************************************
 * Función : tcp_server_recon_cb
 * @brief  : Callback cuando la comunicación tcp es interrumpida. Indica que la
             comunicación tcp ha sido forzada a cerrarse.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_RC_CB".
 *******************************************************************************/
void tcp_server_recon_cb(void *arg, sint8 err){

    int8_t info_envio;
    unsigned long time0;

    // Se ha producido una fallo, y la conexión ha sido cerrada.
    tcp_desconectado = true;
    tcp_establecido = false;
    transmision_finalizada = true;
    tcp_recibido = false;

    #ifdef _DEBUG_COMUNICACION
        debug.print("[TCP_RC_CB] CONEXION INTERRUMPIDA. CODIGO DE ERROR: ");
        debug.println(err);
    #endif

    /* AÚN NO COMPROBADO */
    info_envio = espconn_disconnect(esp_conn);

    time0 = millis();
    while (!tcp_desconectado) {
      yield();

      if (info_envio != ESPCONN_OK)
         info_envio = espconn_disconnect(esp_conn);

      if ((millis()-time0)>MAX_ESPWIFI){
        return;
      }
    }
    #ifdef _DEBUG_COMUNICACION
      debug.print("[TCP_RC_CB] Comunicacion TCP cerrada. Tiempo requerido: ");
      debug.println(millis()-time0);
    #endif
    /* AÚN NO COMPROBADO */
}

/******************************************************************************
 * Función : tcp_server_recv_cb
 * @brief  : Callback cuando se recibe información del cliente. Permite leer la
             la trama de datos recibida e identificar la operación solicitada.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_RV_CB".
 *******************************************************************************/
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length)
{

  #ifdef _DEBUG_COMUNICACION_LIMIT
      debug.print("[TCP_RV_CB] Recepcion de datos. Numero de datos recibidos: ");
      debug.println(length);
      debug.print("[TCP_RV_CB] Información recibida: ");
      debug.println(tcp_data);
  #endif

    /* PROCESAMIENTO DE LA INFORMACIÓN RECIBIDA */
    switch (tcp_data[0]) {

      case WACK:
        registro_confirmado = true;
        break;

      default:
      #ifdef _DEBUG_COMUNICACION
        debug.println("[TCP_RV_CB]: Operacion no identificado.");
      #endif
      break;

      // Operación de debug.
      #ifdef _DEBUG_COMUNICACION
        case '!':
          Serial.println(tcp_data);
          break;
      #endif
      }

    #ifdef _DEBUG_COMUNICACION
        debug.print("TCP RECV: FIN comunicacion.");
    #endif



    tcp_recibido = true;
}

/******************************************************************************
 * Función : tcp_listen
 * @brief  : Callback cuando se establece la comunicación TCP. Permite identificar
             cuando se ha iniciado a la comunicación y establecer las funciones
             de Callback para los distintos eventos de la comunicación TCP.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCPL".
 *******************************************************************************/
void tcp_listen(void *arg)
{
    #ifdef _DEBUG_COMUNICACION
        debug.println("[TCPL] Comunicacion iniciada");
    #endif
    struct espconn *pesp_conn = static_cast<struct espconn *> (arg);

   /* Función llamada cuando se reciben datos */
   espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
   /* Función llamada cuando la conexión es interrumpida */
   espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
   /* Función llamada cuando se finaliza la conexión */
   espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);
   /* Función llamada cuando los datos se han enviado correctamente */
   espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);

   tcp_establecido = true;
   tcp_desconectado = false;
   transmision_finalizada = true;
   tcp_recibido = false;

}

/**
 * Establece la configuración de la red Wifi, la cual será utilizada para
 * actuar como cliente de un servidor.
 * Parametros por defecto de la red para AP:
 * IP: 192.168.1.100, Puerta de enlace: 192.168.1.255, Mascara: 255.255.255.0
 * SSID: MCESP_'IDCHIP', contraseña: zxcvmnbv1234
 * Parametros para sincronizacion de dispositivos:
 * IP: 192.168.1.100, Puerta de enlace: 192.168.1.255, Mascara: 255.255.255.0
 * SSID: MCESP_SINCRONIZANDO, contraseña: zxcvmnbv1234
**/
/******************************************************************************
 * Función : configWifi
 * @brief  : Establece la configuración de la red Wifi, la cual será utilizada para
              actuar como cliente de un servidor. Parametros por defecto de la red para AP:
              IP: 192.168.1.0, Puerta de enlace: 192.168.1.255, Mascara: 255.255.255.0
              SSID: MCESP_'IDCHIP', contraseña: zxcvmnbv1234.
 * @param  : none
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "CONFW".
 *******************************************************************************/
void configWifi(){

  String st_ssid;
  struct softap_config *config = (struct softap_config *)malloc(sizeof(struct
  softap_config));
  struct ip_info info;
  struct dhcps_lease dhcp_lease;

  wifi_set_opmode(SOFTAP_MODE);

  // Se establece la Red Wifi donde se establecerá el módulo como servidor
  st_ssid = String(  String( PRE_SSID ) +  String( ESP.getChipId() ) );

  uint8_t len = st_ssid.length();
  char ssid[len];
  st_ssid.toCharArray(ssid,len+1);

  // Se obtiene la configuración previamente cargada.
  wifi_softap_get_config(config);
  strcpy(reinterpret_cast<char*>(config->ssid),ssid);
  strcpy(reinterpret_cast<char*>(config->password), CONTRASENA);
  config->authmode = AUTH_WPA_WPA2_PSK;
  config->max_connection = MAX_USUARIOS;
  config->ssid_hidden = HIDDEN_DEFAULT;
  config->beacon_interval = BEACON_INTERVAL;

  // Las nuevas modificaciones son grabadas en la memoria flash.
  wifi_softap_set_config_current(config);
  free(config);

  // Se establece la configuracion del DHCP
  wifi_softap_dhcps_stop();  // disable soft-AP DHCP server
  IP4_ADDR(&info.ip, 192, 168, 1, 0); // set IP
  IP4_ADDR(&info.gw, 192, 168, 1, 255); // set gateway
  IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
  wifi_set_ip_info(SOFTAP_IF, &info);

  // Se india el rango de ip que puede ofrecer el servicio de DHCP
  IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 1, 1);
  IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 1, 254);
  wifi_softap_set_dhcps_lease(&dhcp_lease);
  wifi_softap_dhcps_start(); // enable soft-AP DHCP server

  #ifdef _DEBUG_WIFI
    debug.print("[CONFW] SSID: ");
    debug.println(st_ssid);
    for(int i = 0; i < len; i++){
      debug.print(ssid[i]);
    }

    debug.println("\n[CONFW] WiFi establecida");
    debug.println("[CONFW] IP address: ");
    debug.println(WiFi.softAPIP());
  #endif

  esp_conn = (struct espconn *)os_malloc((uint32)sizeof(struct espconn));
  esp_conn->type = ESPCONN_TCP;
  esp_conn->state = ESPCONN_NONE;
  esp_conn->proto.tcp = (esp_tcp *)os_malloc((uint32)sizeof(esp_tcp));
  esp_conn->proto.tcp->remote_port = MCPESP_SERVER_PORT;


  return;
}

/**
**/
bool conexion_servidor(const uint32_t host, bool check_conex){

  union {
    uint32_t value;
    uint8_t byte[4];
  } _host;

  int8_t res_con;
  unsigned long time0;


  #ifdef _DEBUG_COMUNICACION_CONEXION
    debug.print("CONEXION SERVIDOR: Direccion ip cliente actual: ");
    debug.println(host,HEX);
    debug.print("CONEXION SERVIDOR: Direccion ip cliente anterior: ");
    debug.println(prev_host,HEX);
  #endif

  if ( host == prev_host || ((uint32_t) host & 0x00FFFFFF) != 0x0001A8C0){ // XX01A8C0 = 192.168.1.xxx

  /*  remot_info *info_esp_con = NULL;
    int8_t ret = espconn_get_connection_info(esp_conn,&info_esp_con,0);
    #ifdef _DEBUG_COMUNICACION_CONEXION
      debug.print("CONEXION SERVIDOR: Servidor conectado. Valor de get_connection_info: ");
      debug.println(ret);
      debug.println(info_esp_con->state);
    #endif
    if (info_esp_con->state != ESPCONN_NONE && info_esp_con->state != ESPCONN_CLOSE)
  */
    if (!check_conex)
      return false;

  }

  if (host != prev_host && (prev_host != 0) && ((uint32_t) host & 0x00FFFFFF) == 0x0001A8C0 ) {

#ifdef _DEBUG_COMUNICACION_CONEXION
    debug.println("CONEXION SERVIDOR: Previa desconexion del comunicacion con el servidor anterior.");
    debug.println("CONEXION SERVIDOR: Se envia codigo de desconexion.");
#endif

    uint8_t psent[1];
    psent[0] = '#';
    res_con = espconn_send(esp_conn, psent , 1);

    time0 = millis();
    while (!tcp_desconectado && !transmision_finalizada) {
      yield();

      if (res_con != ESPCONN_OK)
         res_con = espconn_send(esp_conn, psent , 1);

      if ((millis()-time0)>MAX_ESPWIFI){
        return false;
      }
    }

#ifdef _DEBUG_COMUNICACION
   debug.print("REGISTRO_CONEXION: Comunicacion TCP cerrada. Tiempo requerido: ");
   debug.println(millis()-time0);
#endif

/*
    sint8_t ret = espconn_disconnect(esp_conn);

    debug.println("espconn_disconnect: ");
    debug.println(ret);

    ret = espconn_delete(esp_conn);

    debug.println("espconn_delete: ");
    debug.println(ret);
*/
/*
    ret = espconn_set_opt(esp_conn,ESPCONN_REUSEADDR);

    debug.println("espconn_set_opt: ");
    debug.println(ret);
*/
//   os_free(esp_conn);

  }

  prev_host = host;

 // if (!mcpesp_server.connected()) {

#ifdef _DEBUG_COMUNICACION_CONEXION
  debug.println("CONEXION SERVIDOR: Intentado establecer conexion con el servidor.");
  time0 = millis();
#endif

//  esp_conn = (struct espconn *)os_malloc((uint32)sizeof(struct espconn));

  _host.value = host;
  esp_conn->type = ESPCONN_TCP;
  esp_conn->state = ESPCONN_NONE;
//  esp_conn->proto.tcp = (esp_tcp *)os_malloc((uint32)sizeof(esp_tcp));
  esp_conn->proto.tcp->remote_port = MCPESP_SERVER_PORT;
  esp_conn->proto.tcp->remote_ip[0] = _host.byte[0];
  esp_conn->proto.tcp->remote_ip[1] = _host.byte[1];
  esp_conn->proto.tcp->remote_ip[2] = _host.byte[2];
  esp_conn->proto.tcp->remote_ip[3] = _host.byte[3];
  espconn_regist_connectcb(esp_conn, tcp_listen);


#ifdef _DEBUG_COMUNICACION_CONEXION
  res_con = espconn_connect(esp_conn);

  debug.print("CONEXION SERVIDOR: Aviso de conexion: ");
  debug.println(res_con);

  time0 = millis();
  while(res_con != ESPCONN_OK){
    debug.print("CONEXION SERVIDOR: Estableciendo conexión. Tiempo requerido: ");
    debug.println(millis()-time0);

    yield();

    if (res_con == ESPCONN_ISCONN)
      return true;

    res_con = espconn_connect(esp_conn);

    if ((millis()-time0)>MAX_ESPWIFI){
      debug.println("CONEXION SERVIDOR: No establecida.");
      return false;
    }
  }
#else
  time0 = millis();
  while(espconn_connect(esp_conn) != ESPCONN_OK ){
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }
#endif

#ifdef _DEBUG_COMUNICACION_CONEXION
    debug.println("CONEXION SERVIDOR: Establecida.");
#endif

    return true;
}

/**
**/
void comunicacion_servidor(){

    unsigned long time0;

    if (!transmision_finalizada || tcp_desconectado){
      return;
    }

    #ifdef _DEBUG_COMUNICACION_LIMIT
      debug.println("COMUNICACION SERVIDOR: INICIO comunicacion.");
    #endif

    transmision_finalizada = false;

    uint8_t psent[1];
    psent[0] = '!';

  #ifdef _DEBUG_COMUNICACION
    int8_t res_envio = espconn_send(esp_conn, psent , 1);
    time0 = millis();
    while (res_envio != ESPCONN_OK){
      yield();

      if (res_envio != ESPCONN_OK)
         res_envio = espconn_send(esp_conn, psent , 1);

         debug.print("COMUNICACION SERVIDOR: Codigo de envio: ");
         debug.println(res_envio);

      if ((millis()-time0)>MAX_ESPWIFI){
         return;
       }
    }
  #else
    time0 = millis();
    while (espconn_send(esp_conn, psent , 1) != ESPCONN_OK){
      yield();
      if ((millis()-time0)>MAX_ESPWIFI){
        return;
      }
    }
  #endif

  #ifdef _DEBUG_COMUNICACION_LIMIT
      debug.print("COMUNICACION SERVIDOR: Cliente conectado. Timecounter: ");
      debug.println(timecounter);
  #endif

}

/**
**/
bool confirmar_conexion(uint32_t host){

  int8_t res_envio;
  unsigned long time0;

  // Se indica previamente que el usuario no se encuentra conectado.
  registro_confirmado = false;

  #ifdef _DEBUG_COMUNICACION
  debug.print("REGISTRO_CONEXION: Se intenta establecer conexion con el servidor: ");
  debug.println(host,HEX);
  #endif
  if (!conexion_servidor(host,true)){
    prev_host = 0;
    return false;
  }
  #ifdef _DEBUG_COMUNICACION
  debug.println("REGISTRO_CONEXION: Conexion establecida?");
  #endif

  time0 = millis();
  while (!tcp_establecido) {
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }

  if (tcp_establecido){

//    tcp_establecido = false;
//    tcp_desconectado = false;

    #ifdef _DEBUG_COMUNICACION
    debug.print("REGISTRO_CONEXION: Si, la conexion se ha establecido. Tiempo requerido: ");
    debug.println(millis()-time0);
    debug.println("REGISTRO_CONEXION: Se envia respuesta de confirmacion de registro.");
    time0 = millis();
    #endif

   uint8_t psent[1];
   psent[0] = USUARIO_REGISTRADO;

   time0 = millis();
   #ifdef _DEBUG_COMUNICACION
   res_envio = espconn_send(esp_conn, psent , 1);

  while (!transmision_finalizada){
     yield();

     if (res_envio != ESPCONN_OK)
        res_envio = espconn_send(esp_conn, psent , 1);

     debug.print("REGISTRO_CONEXION: Codigo de envio: ");
     debug.println(res_envio);

     if ((millis()-time0)>MAX_ESPWIFI){
       return false;
     }
   }
   #else
    res_envio = espconn_send(esp_conn, psent , 1);

    while (!transmision_finalizada){
       yield();

       if (res_envio != ESPCONN_OK)
          res_envio = espconn_send(esp_conn, psent , 1);

       if ((millis()-time0)>MAX_ESPWIFI){
         return false;
       }
     }
     #endif


     #ifdef _DEBUG_COMUNICACION
   debug.print("REGISTRO_CONEXION: Se ha enviado codigo de registro. Tiempo requerido: ");
   debug.println(millis()-time0);
   debug.println("REGISTRO_CONEXION: A espera de la confirmacion de la transmision.");
   #endif

   time0 = millis();
   while (!registro_confirmado) {
     yield();
     if ((millis()-time0)>MAX_ESPWIFI){
       return false;
     }
   }

 #ifdef _DEBUG_COMUNICACION
   debug.print("REGISTRO_CONEXION: Confirmacion del registro. Tiempo requerido: ");
   debug.println(millis()-time0);
   debug.println("REGISTRO_CONEXION: A espera del cierre de la comunicacion.");
 #endif

  psent[0] = '#';
  transmision_finalizada = false;

  time0 = millis();
  #ifdef _DEBUG_COMUNICACION
  res_envio = espconn_send(esp_conn, psent , 1);

   while (!transmision_finalizada){
      yield();

      if (res_envio != ESPCONN_OK)
         res_envio = espconn_send(esp_conn, psent , 1);

      debug.print("REGISTRO_CONEXION: Codigo de envio: ");
      debug.println(res_envio);

      if ((millis()-time0)>MAX_ESPWIFI){
        return false;
      }
    }
  #else
     res_envio = espconn_send(esp_conn, psent , 1);

     while (!transmision_finalizada){
        yield();

        if (res_envio != ESPCONN_OK)
           res_envio = espconn_send(esp_conn, psent , 1);

        if ((millis()-time0)>MAX_ESPWIFI){
          return false;
        }
      }
  #endif

  time0 = millis();
  while (!tcp_desconectado) {
    yield();

    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }
  prev_host = 0;

  #ifdef _DEBUG_COMUNICACION
     debug.print("REGISTRO_CONEXION: Comunicacion TCP cerrada. Tiempo requerido: ");
     debug.println(millis()-time0);
     #endif

  }

  if (registro_confirmado) {
    //    registro_confirmado = false;
    #ifdef _DEBUG_COMUNICACION
       debug.println("REGISTRO_CONEXION: ------------------------------------------------------------------");
       debug.print("REGISTRO_CONEXION: Sincronizacion realizada con exito. Tiempo requerido:");
       debug.println(millis()-time0);
       debug.println("REGISTRO_CONEXION: ------------------------------------------------------------------");
    #endif
    return true;
  } else {
    #ifdef _DEBUG_COMUNICACION
      debug.println("REGISTRO_CONEXION: Sincronizacin fallida");
    #endif
    return false;
  }
}

/******************************************************************************
 * Función : guardar_red
 * @brief  : Lee de la memoria EEPROM los usuarios registrados y los registra en
             la estructura de datos "red".
 * @param  : red - puntero de la estructura de datos donde está el número
              de usuarios registrados y su correspondiente MAC.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "MGR".
 *******************************************************************************/
void tcp_comunication(const uint32_t host){

  union {
    uint32_t value;
    uint8_t byte[4];
  } _host;

  int8_t res_con;
  unsigned long time0;

  if (!transmision_finalizada || !tcp_desconectado){
    return;
  }

  #ifdef _DEBUG_COMUNICACION_CONEXION
    debug.print("TCP_C: conexion con el servidor (direccion ip): ");
    debug.println(host,HEX);
  #endif

  #ifdef _DEBUG_COMUNICACION_CONEXION
    debug.println("TCP_C: Intentado establecer conexion con el servidor.");
    time0 = millis();
  #endif

    _host.value = host;
    esp_conn->proto.tcp->remote_ip[0] = _host.byte[0];
    esp_conn->proto.tcp->remote_ip[1] = _host.byte[1];
    esp_conn->proto.tcp->remote_ip[2] = _host.byte[2];
    esp_conn->proto.tcp->remote_ip[3] = _host.byte[3];
    espconn_regist_connectcb(esp_conn, tcp_listen);


  #ifdef _DEBUG_COMUNICACION_CONEXION
    res_con = espconn_connect(esp_conn);

    debug.print("TCP_C: Aviso de conexion: ");
    debug.println(res_con);

    time0 = millis();
    while(res_con != ESPCONN_OK){
      debug.print("TCP_C: Estableciendo conexión. Tiempo requerido: ");
      debug.println(millis()-time0);

      yield();

      if (res_con == ESPCONN_ISCONN)
        break;

      res_con = espconn_connect(esp_conn);

      if ((millis()-time0)>MAX_ESPWIFI){
        debug.println("TCP_C: No establecida.");
        return;
      }
    }
  #else
    time0 = millis();
    while(espconn_connect(esp_conn) != ESPCONN_OK ){
      yield();
      if ((millis()-time0)>MAX_ESPWIFI){
        return false;
      }
    }
  #endif

  time0 = millis();
  while (!tcp_establecido) {
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return;
    }
  }

  #ifdef _DEBUG_COMUNICACION_CONEXION
      debug.print("TCP_C: Conexion establecida. Tiempo requerido: ");
      debug.println(millis()-time0);
      debug.println("TCP_C: INICIO de la comunicacion.");
    #endif

    // Trama de datos que se desea enviar al servidor a través de tcp.
    uint8_t psent[1];
    psent[0] = '!';
    transmision_finalizada = false;

  #ifdef _DEBUG_COMUNICACION
    int8_t res_envio = espconn_send(esp_conn, psent , 1);
    time0 = millis();
    while (res_envio != ESPCONN_OK){
      yield();

      if (res_envio != ESPCONN_OK)
         res_envio = espconn_send(esp_conn, psent , 1);

         debug.print("TCP_C: Codigo de envio: ");
         debug.println(res_envio);

      if ((millis()-time0)>MAX_ESPWIFI){
         return;
       }
    }
  #else
    time0 = millis();
    while (espconn_send(esp_conn, psent , 1) != ESPCONN_OK){
      yield();
      if ((millis()-time0)>MAX_ESPWIFI){
        return;
      }
    }
  #endif

  time0 = millis();

  while (!transmision_finalizada){
     yield();

     if (res_envio != ESPCONN_OK)
        res_envio = espconn_send(esp_conn, psent , 1);

     if ((millis()-time0)>MAX_ESPWIFI){
       return;
     }
   }

  #ifdef _DEBUG_COMUNICACION_CONEXION
     debug.print("TCP_C: Transmision realizada. Tiempo requerido: ");
     debug.println(millis()-time0);
  #endif

  time0 = millis();
  while (!tcp_recibido){
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      break;
    }
  }
  tcp_recibido = false;

  #ifdef _DEBUG_COMUNICACION_CONEXION
     debug.print("TCP_C: Información recepcionada. Tiempo requerido: ");
     debug.println(millis()-time0);
     debug.println("TCP_C: Previa desconexion del comunicacion con el servidor anterior.");
     debug.println("TCP_C: Se envia codigo de desconexion.");
  #endif
/*
      psent[0] = '#';
      res_con = espconn_send(esp_conn, psent , 1);
      transmision_finalizada = false;
      time0 = millis();

      while (!transmision_finalizada){
         yield();

         if (res_envio != ESPCONN_OK)
            res_envio = espconn_send(esp_conn, psent , 1);

         if ((millis()-time0)>MAX_ESPWIFI){
           return;
         }
       }
*/
       res_envio = espconn_disconnect(esp_conn);

       time0 = millis();
       while (!tcp_desconectado) {
         yield();

         if (res_envio != ESPCONN_OK)
            res_envio = espconn_disconnect(esp_conn);

         if ((millis()-time0)>MAX_ESPWIFI){
           return;
         }
       }


  #ifdef _DEBUG_COMUNICACION
     debug.print("TCP_C:: Comunicacion TCP cerrada. Tiempo requerido: ");
     debug.println(millis()-time0);
  #endif

}
