Codigo fuente de la practica (el que uso el equipo en la prueba del video).

  Inclinometro_Motor_R4/Inclinometro_Motor_R4.ino
    setup(): salidas del motor en BAJO -> I2C a 100 kHz con timeout de 5 ms
    -> WHO_AM_I (0x75) debe ser 0x68 -> PWR_MGMT_1 = 0x01 (despierta, reloj
    PLL), +-250 grados/s, +-2 g, filtro interno ~44 Hz, 100 muestras/s ->
    calibra el giroscopio Y con 500 lecturas (~5 s, sensor QUIETO) -> angulo
    inicial con el acelerometro. Si algo falla: estado BLOQUEADO, motor
    apagado, X en la matriz.
    loop() sin delay(), 4 tareas con millis():
      10 ms  lee 14 bytes desde 0x3B, filtro complementario con
             alfa = tau/(tau+dt), tau = 0.5 s (~0.98 a 100 Hz), dt con micros()
      20 ms  rampa de 9 PWM por paso (580 ms de 0 a 255); al invertir, primero a 0
      500 ms Monitor Serie solo si cambio; ENTRA/SALE DEL CENTRO (+-2 grados)
      50 ms  matriz: punto 2x2 (arriba = adelante), marco en el centro, X en paro
    Failsafe: lectura fallida o dt > 100 ms -> PARO DE SEGURIDAD (motor a 0 sin
    rampa); reintenta cada 250 ms y al responder "SENSOR RECUPERADO".

Ajustes: ZONA_MUERTA 5, ZONA_CENTRO 2, ANGULO_MAXIMO 45, PWM_MINIMO 90,
PASO_RAMPA 9, TAU_FILTRO 0.5, SENTIDO_PITCH (-1.0 si el angulo sale invertido).
Recomendado: resistencia de 10 kohm entre ENA y GND.

Compilado con arduino-cli para "arduino:renesas_uno:unor4wifi": 25% de programa.
