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
 * Etiqueta debug : Todos los comentarios para depuración de esta función
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
 * Etiqueta debug : Todos los comentarios para depuración de esta función
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
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_RC_CB".
 *******************************************************************************/
void tcp_server_recon_cb(void *arg, sint8 err){

    int8_t info_tcp;
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
    info_tcp = espconn_disconnect(esp_conn);

    time0 = millis();
    while (!tcp_desconectado) {
      yield();

      if (info_tcp != ESPCONN_OK)
         info_tcp = espconn_disconnect(esp_conn);

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
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_RV_CB".
  * ------------------------ *
   Protocolo de comunicación
  * ------------------------ *

 |------------|--------------|---------------|---------|--------|-------------------|
 | -- Start --|-- tcpcount --|-- ident_var --|-- Var --|- \...\-|-- Stop/Continue --|
 |------------|--------------|---------------|---------|--------|-------------------|

 Start (start) - uint8_t = ¿  || Byte de inicio de comunicación.
 tcpcount      - uint8_t =    || Número de variables que serán recibidas.
 ident_var     - *uint8_t =   || Identificador de la variable recibida.
 Var           - *double      || Variable
 Stop/Continue - uint8_t = #/?|| Byte de fin de comunicación o indicador de mantener la comunicación.

 *******************************************************************************/
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length)
{

  union {
    float float_value;
    uint64_t long_value;
    uint8_t byte[8];
  } var;

  uint8_t lon = 0;
  uint8_t start = tcp_data[0];
  uint8_t tcpcount = tcp_data[1];

  stop_continue = tcp_data[length-1];

  #ifdef _DEBUG_COMUNICACION
      debug.print("[TCP_RV_CB] Recepcion de datos. Numero de datos recibidos: ");
      debug.println(length);
      debug.print("[TCP_RV_CB] Información recibida: ");
      debug.println(tcp_data);
  #endif

  /* PROCESAMIENTO DE LA INFORMACIÓN RECIBIDA */
  //       j =    // Variable auxiliar para recorrer la trama de datos.
                  // Los datos se empiezan a recibir a partir de 3 byte.
                  // El conjunto de bytes que identifican y definen su valor
                  // esta formado por 5 bytes.

  for (int j = 2; j < ( 2 + (tcpcount)*5 ) ; j += 5){
  //  if (tcp_data[j] == TCP_U64)
  //      lon = 64;

      for (uint8_t i = 0; i < 8 + lon; i++)
        var.byte[i] = tcp_data[3+i];

    switch (tcp_data[j]) {

      case WACK:
        registro_confirmado = true;
        break;

      default:
      #ifdef _DEBUG_COMUNICACION
        debug.println("[TCP_RV_CB] Operacion no identificado.");
      #endif
      break;

      // Operación de debug.
    //  #ifdef _DEBUG_COMUNICACION
        case '!':

          /*
          debug.print("[TCP_RV_CB] Recepcion de datos. Numero de datos recibidos: ");
          debug.println(length);
          debug.print("Datos recibidos: ");
          debug.println(tcp_data);
          debug.print("Start: ");
          debug.println(start);
          debug.print("tpcount: ");
          debug.println(tcpcount);
          debug.print("ident_var: ");
          debug.println(tcp_data[j]);
          debug.print("var: ");
          debug.printLLNumber(var.long_value,10);
          debug.print("Stop: ");
          debug.println(stop_continue);
          */


          debug.print("Instante de tiempo de sincronizacion: ");
          debug.printLLNumber(((usuario_conectado->time_sync) / 10000000) / 100,10);
          debug.println();
          debug.print("Tiempo transcurrido desde la sincronizacion: ");
          uint64_t t = ( ( get_rtc_time()-usuario_conectado->time_sync) / 10000000);
          debug.printLLNumber( t/100,10);
          debug.print(".");
          debug.printLLNumber( t % 100,10);
          debug.println();
          debug.print("Tiempo indicado por el servidor ");
          t = ( ( var.long_value ) / 10000000);
          debug.printLLNumber( t/100,10);
          debug.print(".");
          debug.printLLNumber( t % 100,10);
          debug.println();
          break;
    //  #endif
      }

      // Se incremente la variable, para apuntar el siguiente datos en la trama.
      j += 5;
  }

  tcp_recibido = true;

    #ifdef _DEBUG_COMUNICACION
        debug.println("[TCP_RV_CB] FIN comunicacion.");
    #endif
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

/******************************************************************************
 * Función : tcp_comunication
 * @brief  : Establece una comunicación TCP con el dispositivo indicado. Una vez establecido
              el canal de comunicación se le solicita la información deseada y se finaliza la comunicación.
 * @param  : host - IP del usuario con el cual se desea establecer la comunicación TCP.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCPCM".
 *******************************************************************************/
void tcp_comunication(const uint32_t host){

  union {
    uint32_t value;
    uint8_t byte[4];
  } _host;

  int8_t info_tcp;
  unsigned long time0;

  if (!transmision_finalizada || !tcp_desconectado){
    return;
  }

  //if (stop_continue == TCP_CONTINUE)
  //  return;

  #ifdef _DEBUG_COMUNICACION
    debug.print("[TCPCM] conexion con el servidor (direccion ip): ");
    debug.println(host,HEX);
  #endif

  #ifdef _DEBUG_COMUNICACION
    debug.println("[TCPCM] Intentado establecer conexion con el servidor.");
    time0 = millis();
  #endif

    _host.value = host;
    esp_conn->proto.tcp->remote_ip[0] = _host.byte[0];
    esp_conn->proto.tcp->remote_ip[1] = _host.byte[1];
    esp_conn->proto.tcp->remote_ip[2] = _host.byte[2];
    esp_conn->proto.tcp->remote_ip[3] = _host.byte[3];
    espconn_regist_connectcb(esp_conn, tcp_listen);


  #ifdef _DEBUG_COMUNICACION
    info_tcp = espconn_connect(esp_conn);

    debug.print("[TCPCM] Aviso de conexion: ");
    debug.println(info_tcp);

    time0 = millis();
    while(info_tcp != ESPCONN_OK){
      debug.print("[TCPCM] Estableciendo conexión. Tiempo requerido: ");
      debug.println(millis()-time0);

      yield();

      if (info_tcp == ESPCONN_ISCONN)
        break;

      info_tcp = espconn_connect(esp_conn);

      if ((millis()-time0)>MAX_ESPWIFI){
        debug.println("[TCPCM] No establecida.");
        return;
      }
    }
  #else
    time0 = millis();
    while(espconn_connect(esp_conn) != ESPCONN_OK ){
      yield();
      if ((millis()-time0)>MAX_ESPWIFI){
        return;
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

  #ifdef _DEBUG_COMUNICACION
      debug.print("[TCPCM] Conexion establecida. Tiempo requerido: ");
      debug.println(millis()-time0);
      debug.println("[TCPCM] INICIO de la comunicacion.");
    #endif

    // Trama de datos que se desea enviar al servidor a través de tcp.
    uint8_t psent[1];
    psent[0] = '!';
    transmision_finalizada = false;

  #ifdef _DEBUG_COMUNICACION
    info_tcp = espconn_send(esp_conn, psent , 1);
    time0 = millis();
    while (info_tcp != ESPCONN_OK){
      yield();

      if (info_tcp != ESPCONN_OK)
         info_tcp = espconn_send(esp_conn, psent , 1);

         debug.print("[TCPCM] Codigo de envio: ");
         debug.println(info_tcp);

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

     if ((millis()-time0)>MAX_ESPWIFI){
       return;
     }
   }

  #ifdef _DEBUG_COMUNICACION
     debug.print("[TCPCM] Transmision realizada. Tiempo requerido: ");
     debug.println(millis()-time0);
  #endif

  time0 = millis();
  while (!tcp_recibido){
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
    //  stop_continue == TCP_STOP;
      return;
    }
  }

  #ifdef _DEBUG_COMUNICACION
    if (tcp_recibido){
      debug.print("[TCPCM] Informacion recepcionada. Tiempo requerido: ");
      debug.println(millis()-time0);
    }
      debug.println("[TCPCM] Desconexion del servidor anterior.");
  #endif

  tcp_recibido = false;

//  if (stop_continue == TCP_STOP){
   info_tcp = espconn_disconnect(esp_conn);

   time0 = millis();
   while (!tcp_desconectado) {
     yield();

     if (info_tcp != ESPCONN_OK)
        info_tcp = espconn_disconnect(esp_conn);

     if ((millis()-time0)>MAX_ESPWIFI){
       return;
     }
   }

    #ifdef _DEBUG_COMUNICACION
       debug.print("[TCPCM] Comunicacion TCP cerrada. Tiempo requerido: ");
       debug.println(millis()-time0);
    #endif
//  }

}
