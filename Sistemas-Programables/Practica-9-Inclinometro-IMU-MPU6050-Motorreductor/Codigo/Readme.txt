Codigo fuente de la practica (escrito siguiendo la consigna 3.2.3).

  InclinometroMotor/InclinometroMotor.ino
    setup(): motor apagado -> WHO_AM_I (0x75) debe ser 0x68, si no se
    detiene -> despierta el sensor (PWR_MGMT_1 = 0), +-2 g y +-250 grados/s
    -> calibra el giroscopio Y con 500 lecturas -> angulo inicial con el
    acelerometro.
    loop() sin delay(), 4 tareas con millis():
      10 ms  lee 14 bytes desde 0x3B, filtro complementario (0.98 / 0.02),
             velocidad objetivo: zona muerta +-5, PWM 90..255 entre 5 y 45
      20 ms  rampa de 9 PWM por paso (~0.6 s de 0 a 255), aplica IN1/IN2/ENA
      500 ms Monitor Serie (solo si cambio): direccion, grados, intensidad,
             motor y %; avisa una vez al entrar/salir del centro (+-2)
      50 ms  matriz: punto 2x2 (arriba = adelante), marco completo en +-2,
             X en paro de seguridad
    Failsafe: si falla la lectura I2C el motor va a 0 sin rampa, imprime
    "PARO DE SEGURIDAD" una vez y reintenta configurar el sensor en cada
    ciclo; al responder, se recupera solo.

Ajustes en el codigo:
  ZONA_MUERTA = 5.0, ANG_MAX = 45.0, PWM_MIN = 90, PASO_RAMPA = 9, ALFA = 0.98
  Si el motor gira al reves de lo esperado, intercambiar los cables de OUT1/OUT2
  (o los de IN1/IN2). Si el angulo sale con el signo invertido, el sensor esta
  montado girado 180 grados.

Nota: la direccion del sensor se llama DIR_MPU porque "MPU" a secas choca con
una macro del nucleo ARM de la R4 (la unidad de proteccion de memoria).

Compilado con arduino-cli para "arduino:renesas_uno:unor4wifi": 25% de programa.
