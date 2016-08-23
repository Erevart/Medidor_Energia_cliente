/******************************************************************************
 * Función : update_rtc_time
 * @brief  : Actualiza registro de tiempo de la memoria flash.
 * @param  : reset - variable que indica resetear el instante el contador de tiempo.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "URTC".
 *******************************************************************************/
void update_rtc_time(bool reset){
  RTC_TIMER rtcTime;
  uint32_t rtc, brtc;

  // Se lee los parámetros de la estructura guardados en memoria.
  system_rtc_mem_read(64, &rtcTime, sizeof(rtcTime));

  // Initialise the time struct
  if (rtcTime.magic != RTC_MAGIC || reset)
  {
    #ifdef _DEBUG_RTC
      debug.println( "[URTC] RTC TIME RESET");
    #endif

    rtcTime.magic = RTC_MAGIC;
    rtcTime.timeAcc = 0;
    rtcTime.timeBase = system_get_rtc_time();

    #ifdef _DEBUG_RTC
      debug.print( "[URTC] Base de tiempo: ");
      debug.println( rtcTime.timeBase );
    #endif

    return;
  }

  #ifdef _DEBUG_RTC_TEST
    uint32 rtcT1, rtcT2;
    uint32 st1, st2;
    uint32 cal1, cal2;

    debug.print("[URTC]===================\r\n");
    debug.println("[URTC] RTC time test");
    // Muestra la precisión de las funciones "system_get_rtc_time" y
    // system_get_time determinando el tiempo transcurrido al introducir
    // un delay. Además muestra el tiempo de encendido total del disposito.

    // Inicio del periodo de tiempo a contabilizar.
    cal1 = system_rtc_clock_cali_proc();
    rtcT1 = system_get_rtc_time();
    st1 = system_get_time();

    delayMicroseconds(600);

    // Final del periodo de tiempo a contabilizar.
    rtcT2 = system_get_rtc_time();
    st2 = system_get_time();
    cal2 = system_rtc_clock_cali_proc();

    // Impresión por pantalla de los datos.
    debug.print("[URTC] RTC t2 - t1: ");
    debug.println(rtcT2 - rtcT1);
    debug.print("[URTC] SYS t2 - t1: ");
    debug.println( st2 - st1);
    debug.print("[URTC] RTC base 1: ");
    debug.print( ((cal1 * 1000) >> 12) / 1000);
    debug.print(".");
    debug.println( ((cal1 * 1000) >> 12) % 1000 );
    debug.print("[URTC] RTC base 2: ");
    debug.print( ((cal2 * 1000) >> 12) / 1000);
    debug.print(".");
    debug.println( ((cal2 * 1000) >> 12) % 1000 );

    debug.print("[URTC]===================\r\n");
  #endif


  rtc = system_get_rtc_time();
  brtc = system_rtc_clock_cali_proc();

  // Se calcula el tiempo total que lleva funcionando el dispositivo
  // desde la ultima puesta a cero del registro timeAcc.
  rtcTime.timeAcc += ( ((uint64_t) (rtc - rtcTime.timeBase)) * ((uint64_t) ((brtc * 1000) >> 12)) );

  // Se actualiza la base de tiempo.
  rtcTime.timeBase = brtc;
  system_rtc_mem_write(64, &rtcTime, sizeof(rtcTime));

  #ifdef _DEBUG_RTC
    debug.print("[URTC] RTC precision: ");
    debug.printLLNumber(rtcTime.timeAcc,10);
    debug.println();
    debug.println("[URTC] Tiempo encendido: ");
    debug.print("[URTC] - ");
    debug.printLLNumber( rtcTime.timeAcc / 1000,10);
    debug.println(" us");
    debug.print("[URTC] - ");
    debug.printLLNumber((rtcTime.timeAcc / 10000000) / 100,10);
    debug.print(".");
    debug.printLLNumber( (rtcTime.timeAcc / 10000000) % 100,10);
    debug.println(" s");
    debug.println("[URTC]-----------------------------\r\n");
  #endif

}


/******************************************************************************
 * Función : update_rtc_time
 * @brief  : Proporciona el tiempo transcurrido desde la ultima sincronización
 *            del contador de tiempo.
 * @param  : nose
 * @return : uint64_t devuelve el valor del tiempo transcurrido desde el último reset
 *            del contador de tiempo.
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "GRTC".
 *******************************************************************************/
uint64_t get_rtc_time(){
  RTC_TIMER rtcTime;
  uint32_t rtc, brtc;

  // Se lee los parámetros de la estructura guardados en memoria.
  system_rtc_mem_read(64, &rtcTime, sizeof(rtcTime));

  rtc = system_get_rtc_time();
  brtc = system_rtc_clock_cali_proc();

  // Se calcula el tiempo total que lleva funcionando el dispositivo
  // desde la ultima puesta a cero del registro timeAcc.
  rtcTime.timeAcc += ( ((uint64_t) (rtc - rtcTime.timeBase)) * ((uint64_t) ((brtc * 1000) >> 12)) );

  // Se actualiza la base de tiempo.
  rtcTime.timeBase = brtc;
  system_rtc_mem_write(64, &rtcTime, sizeof(rtcTime));

  #ifdef _DEBUG_RTC
    debug.println("[GRTC]-----------------------------\r\n");
    debug.print("[GRTC] RTC precision: ");
    debug.printLLNumber(rtcTime.timeAcc,10);
    debug.println();
    debug.println("[GRTC] Tiempo encendido: ");
    debug.print("[GRTC] - ");
    debug.printLLNumber( rtcTime.timeAcc / 1000,10);
    debug.println(" us");
    debug.print("[GRTC] - ");
    debug.printLLNumber((rtcTime.timeAcc / 10000000) / 100,10);
    debug.print(".");
    debug.printLLNumber( (rtcTime.timeAcc / 10000000) % 100,10);
    debug.println(" s");
    debug.println("[GRTC]-----------------------------\r\n");
  #endif


  return rtcTime.timeAcc;
}
