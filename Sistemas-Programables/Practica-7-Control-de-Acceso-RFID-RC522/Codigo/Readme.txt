Codigo fuente de la practica.

  ControlAccesoRFID/ControlAccesoRFID.ino
    Inicializa el bus SPI y el lector RC522 y confirma en el Monitor Serie si
    hay comunicacion (lee el registro de version del lector). Imprime el UID
    de cada tarjeta en hexadecimal ("UID: 3A F2 1C 7B") y lo compara contra
    la lista de tarjetas autorizadas:
      - autorizada:    "ACCESO CONCEDIDO", LED verde (D7) + matriz "ACEPTADO"
      - no autorizada: "ACCESO DENEGADO",  LED rojo  (D6) + matriz "RECHAZADO"
    Cada LED tiene su propio temporizador (struct Led) y se apaga solo a los
    2000 ms con millis(). El texto de la matriz avanza una columna cada 60 ms
    sin SCROLL_LEFT, asi que el programa nunca se bloquea.   [LISTO]

UIDs autorizados en el codigo:
  llavero  99 EB 7B 63
  tarjeta  AA 25 60 E4

Librerias (Library Manager del Arduino IDE):
  - MFRC522          -> se instala
  - ArduinoGraphics  -> se instala (texto en la matriz)
  - SPI y Arduino_LED_Matrix vienen con el core de la R4

Compilado con arduino-cli para "arduino:renesas_uno:unor4wifi":
26% de programa.
