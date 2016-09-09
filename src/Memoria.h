
 /******************************************************************************
  * Función : nvrRead_u8
  * @brief  : Escribe una variable de 8 bits en la memoria.
  * @param  : memaddr - dirección de memoria a escribir.
  * @param  : value - dato a guardar en la dirección indicada .
  * @return : none
  *******************************************************************************/
void nvrWrite_u8(uint8_t value, unsigned int memaddr) {

    EEPROM.write(memaddr, value);
}

 /******************************************************************************
  * Función : nvrRead_u8
  * @brief  : Lee una variable de 8 bits en la memoria
  * @param  : memaddr - direccion de memoria a leer.
  * @return : devuelve la información guardada en la dirección de memoria indicada.
  *******************************************************************************/
uint8_t nvrRead_u8(unsigned int memaddr) {

    return EEPROM.read(memaddr);
}

/******************************************************************************
 * Función : saveFlash
 * @brief  : Guarda en la memoria Flash los usuarios registrados.
 * @param  : red - puntero de la estructura de datos donde está el número
              de usuarios registrados y su correspondiente MAC.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "MGR".
 *******************************************************************************/
void saveFlash(lista_usuarios *red){

  if ( (red->numusu == 0) || red->numusu == nvrRead_u8(1) ){
  #ifdef _DEBUG_MEMORIA
      debug.println("[MGR] No hay usuarios registrados o nuevos. No se guarda nada.");
  #endif
    return;
  }

  // Variable para manejar la información de los diferntes usuarios
  infousu **usuario_actual = &red->usuarios;
  unsigned int memaddr = 0;

  #ifdef _DEBUG_MEMORIA
    debug.println("[MGR] Comienza la escritura en memoria");
  #endif

  // La primera dirección en memoria indica si hay datos guardados.
  nvrWrite_u8(DATOS_WIFI,memaddr);
  memaddr = memaddr + 1;

  // La segunda dirección en memoria indica el número de usuarios guardados.
  nvrWrite_u8(red->numusu,memaddr);
  memaddr = memaddr + 1;

  #ifdef _DEBUG_MEMORIA
    debug.println("[MGR] Ya se ha guardado el numero de usuarios existentes");
  #endif

  // Se guarda la información de los usuarios
  while (*(usuario_actual) != NULL){

    for (int j = 0; j < 6; j++){
      nvrWrite_u8((*usuario_actual)->bssid[j], memaddr);
      memaddr = memaddr + 1;
      #ifdef _DEBUG_MEMORIA
        debug.print("[MGR] MAC: ");
        debug.print((*usuario_actual)->bssid[j],HEX);
        debug.print(":");
      #endif
    }
    usuario_actual = &(*usuario_actual)->siguiente;
  }

  #ifdef _DEBUG_MEMORIA
    debug.println();
    debug.println("[MGR] Usuarios guardados en memoria.");
  #endif

  // Se actualiza el bloque de memoria.
  EEPROM.commit();
}

/******************************************************************************
 * Función : lee_red
 * @brief  : Lee de la memoria Flash los usuarios registrados y guardados en
 *           anteriores sesiones. Posteriormente almacena dicha información
 *           en la memoria RAM.
 * @param  : red - puntero de la estructura de datos donde serán guardado el número
              de usuarios registrados y su correspondiente MAC.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "MLR".
 *******************************************************************************/
void readFlash(lista_usuarios *red ){

  infousu *nuevo_usuario;
  infousu **usuario_actual = &red->usuarios;
  unsigned int memaddr = 1;

  red->numusu = nvrRead_u8(memaddr);
  memaddr = memaddr + 1;

  #ifdef _DEBUG_MEMORIA
    debug.print("[MLR] Numero de usuarios: ");
    debug.println(red->numusu);
  #endif

  for (int i = 0; i < red->numusu; i++){

    // Se solicita espacio para una nueva estructura de datos infousu.
    // En el supuesto de no haber espacio se devuelve -1, cualquiero otro caso 0.
    if ((nuevo_usuario = (infousu *) os_malloc (sizeof (infousu))) == NULL)
      return;
    // Se solicita espacio para la variable ip de la estructura.
    if ((nuevo_usuario->ipdir  = (uint32_t) os_malloc (sizeof (uint32_t))) == NULL)
      return;
    // Se solicita espacio para la variable bssid de la estructura
    if ((nuevo_usuario->bssid  = (uint8_t *) os_malloc (6 * sizeof (uint8_t))) == NULL)
      return;

    for (int j = 0; j < 6; j++){
      nuevo_usuario->bssid[j] = nvrRead_u8(memaddr);
      memaddr = memaddr + 1;

      #ifdef _DEBUG_MEMORIA
        debug.print("[MLR] MAC: ");
        debug.print(nuevo_usuario->bssid[j],HEX);
        if (j == 5)
          debug.println();
        else
          debug.print(":");
      #endif
    }
    nuevo_usuario->ipdir = -1;
    nuevo_usuario->estado = false;
    nuevo_usuario->siguiente = NULL;

    *(usuario_actual) = nuevo_usuario;
    usuario_actual = &(*usuario_actual)->siguiente;
  }

}

/******************************************************************************
 * Función : checkFlash
 * @brief  : Comprueba si hay usuarios registrados en memoria
 * @param  : none
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "MCK".
 *******************************************************************************/
void checkFlash(){
  // Se comprueba si se han guardado parametros de usuarios previamente.
    if (nvrRead_u8(0) == DATOS_WIFI){
    #ifdef _DEBUG_MEMORIA
      debug.println("[MCK] Lee de memoria.");
    #endif
      readFlash(&red_usuarios);
    }
    #ifdef _DEBUG_MEMORIA
      else{
          debug.print("[MCK] Dato almacenado en memoria: ");
          debug.println(nvrRead_u8(0),HEX);
          debug.print("[MCK] Usuarios almacenados: ");
          debug.println(nvrRead_u8(1),HEX);
      }
    #endif
}
