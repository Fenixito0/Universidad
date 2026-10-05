Codigo fuente de la practica (el que dio el profesor en la consigna).

  EstacionBMP280/EstacionBMP280.ino
    Busca el sensor en 0x76 y 0x77 leyendo el registro Chip ID (0xD0):
      - 0x58 -> BMP280: lo configura (MODE_NORMAL, temp x2, presion x16,
                filtro IIR x16, standby 500 ms)
      - 0x60 -> BME280: avisa que se use el sketch de Adafruit_BME280
    Cada 1000 ms imprime en formato del Serial Plotter:
      Temp_C:24.31,Presion_Atm_hPa:1004.12,Altura_nivel_mar_m:7.4
    Cada 2500 ms la matriz alterna "24C" / "7m".
    Si la lectura es NaN o la presion sale de 300-1100 hPa: "ERR" en la
    matriz y reintento cada 5000 ms. Todo con millis(), sin delay().   [LISTO]

Ajustes en el codigo:
  SEALEVEL_HPA = 1005.0  -> presion a nivel del mar del dia (para la altitud)
  USE_QWIIC    = 0       -> 1 si se conecta por el conector Qwiic (Wire1)

Librerias (Library Manager del Arduino IDE):
  - Adafruit BMP280 Library
  - Adafruit Unified Sensor
  - ArduinoGraphics (siempre incluir antes de Arduino_LED_Matrix.h)
  - Wire y Arduino_LED_Matrix vienen con el core de la R4

Compilado con arduino-cli para "arduino:renesas_uno:unor4wifi":
31% de programa.
