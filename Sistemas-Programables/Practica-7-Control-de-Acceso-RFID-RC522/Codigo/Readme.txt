Codigo fuente de la practica.

  ControlAccesoRFID/ControlAccesoRFID.ino
    Inicializa el bus SPI y el lector RC522, confirma en el Monitor Serie si
    hay comunicacion con el lector (lee su registro de version), imprime el
    UID de cada tarjeta en hexadecimal ("UID: 3A F2 1C 7B") y decide
    ACCESO PERMITIDO / ACCESO DENEGADO comparando contra el UID guardado.
    El LED (D7) se apaga solo a los 2000 ms usando millis(), asi que el
    programa sigue leyendo tarjetas mientras esta encendido.
    Extra: la matriz LED 12x8 integrada de la R4 WiFi escribe ACEPTADO o
    RECHAZADO desplazandose (sin bloquear el loop) y al terminar deja una
    palomita o una X fija.   [LISTO]

Librerias (Library Manager del Arduino IDE):
  - MFRC522 (GithubCommunity / miguelbalboa)  -> se instala
  - ArduinoGraphics                            -> se instala (texto en la matriz)
  - SPI y Arduino_LED_Matrix                   -> ya vienen con el core de la R4

Compilado con arduino-cli para "arduino:renesas_uno:unor4wifi":
26% de programa, 26% de memoria dinamica.

ANTES DE PROBAR: el UID autorizado viene en 00 00 00 00. Cargar el programa,
acercar la tarjeta y el llavero, anotar sus UID y escribir el de la tarjeta
elegida en UID_AUTORIZADO (linea marcada "PASO 2"). Volver a cargar.

Detalles para la R4:
  - El pin de reset del lector se llama RC522_RST: el nombre PIN_RST ya
    existe en el core de la R4 y no compila (mismo problema que PIN_LED en la
    practica 6).
  - Los dibujos de la matriz no pueden ser "const": renderBitmap() no los
    acepta (error de compilacion verificado).
  - endText(SCROLL_LEFT) de ArduinoGraphics bloquea hasta que termina de
    pasar el texto; por eso el desplazamiento se hace a mano, un paso cada
    70 ms con millis().
