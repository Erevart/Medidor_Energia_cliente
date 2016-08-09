
/* void nvrWrite_u8(uint8_t value, int memaddr)
 Escribe una variable de 8 bits en la memoria */
void nvrWrite_u8(uint8_t value, unsigned int memaddr) {

    EEPROM.write(memaddr, value);
}

/* uint8_t nvrRead_u16(unsigned int memaddr)
 Lee una variable de 8 bits en la memoria */
uint8_t nvrRead_u8(unsigned int memaddr) {

    return EEPROM.read(memaddr);
}

/* void nvrWrite_u24(uint32_t value, int memaddr)
 Escribre una variable de 32 bits en la memoria */
void nvrWrite_u32(uint32_t value, unsigned int memaddr) {

    union u16_Store {
        uint8_t ValByte[4];
        uint16_t Val;
    } ValueIn;

    ValueIn.Val = value;

    EEPROM.write(memaddr, ValueIn.ValByte[0]);
    EEPROM.write(memaddr + 1, ValueIn.ValByte[1]);
    EEPROM.write(memaddr + 2, ValueIn.ValByte[2]);
    EEPROM.write(memaddr + 3, ValueIn.ValByte[3]);
}

void guardar_red(lista_usuarios *red){

  if ( (red->numusu == 0) || red->numusu == nvrRead_u8(1) ){
#ifdef _DEBUG_MEMORIA
    debug.println("MEMORIA: No hay usuarios registrados o nuevos. No se guarda nada.");
#endif
    return;
  }

  infousu **usuario_actual = &red->usuarios;        // Variable para manejar la información de los diferntes usuarios
  unsigned int memaddr = 0;

#ifdef _DEBUG_MEMORIA
  debug.println("MEMORIA: Comienza la escritura en memoria");
#endif

  // Primer dirección en la memoria indica si hay datos guardados.
  nvrWrite_u8(DATOS_WIFI,memaddr);
  memaddr = memaddr + 1;

  nvrWrite_u8(red->numusu,memaddr);
  memaddr = memaddr + 1;

#ifdef _DEBUG_MEMORIA
  debug.println("MEMORIA: Ya se ha guardado el numero de usuarios existentes");
#endif

//  for (int i = 0; i < red_usuarios.numusu; i++){
  while (*(usuario_actual) != NULL){
    for (int j = 0; j < 6; j++){
      nvrWrite_u8((*usuario_actual)->bssid[j], memaddr);
  #ifdef _DEBUG_MEMORIA
      debug.print((*usuario_actual)->bssid[j],HEX);
      debug.print(":");
  #endif
      memaddr = memaddr + 1;
    }
    usuario_actual = &(*usuario_actual)->siguiente;
  }

#ifdef _DEBUG_MEMORIA
    debug.println();
    debug.println("MEMORIA: Usuarios guardados en memoria.");
#endif
    EEPROM.commit();
}

void leer_red(lista_usuarios *red ){

  infousu *nuevo_usuario;
  infousu **usuario_actual = &red->usuarios;
  unsigned int memaddr = 1;

  red->numusu = nvrRead_u8(memaddr);
  memaddr = memaddr + 1;

#ifdef _DEBUG_MEMORIA
  Serial.print("Numero de usuarios: ");
  Serial.println(red->numusu);
#endif

  for (int i = 0; i < red->numusu; i++){

    // Se solicita especio para una nueva estructura de datos infousu.
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
      debug.print(nuevo_usuario->bssid[j],HEX);
      debug.print(":");
      if (j == 5)
        debug.println();
#endif
    }
    nuevo_usuario->ipdir = -1;
    nuevo_usuario->estado = false;
    nuevo_usuario->siguiente = NULL;

    *(usuario_actual) = nuevo_usuario;
    usuario_actual = &(*usuario_actual)->siguiente;
  }

}


void comprobar_eeprom(){
  // Se comprueba si se han guardado parametros de usuarios previamente.
    if (nvrRead_u8(0) == DATOS_WIFI){
#ifdef _DEBUG_MEMORIA
      debug.println("Lee de memoria.");
#endif
      leer_red(&red_usuarios);
    }
#ifdef _DEBUG_MEMORIA
    else{
        debug.print("Dato almacenado en memoria: ");
        debug.println(nvrRead_u8(0),HEX);
        debug.print("Usuarios almacenados: ");
        debug.println(nvrRead_u8(1),HEX);
    }
#endif
}
