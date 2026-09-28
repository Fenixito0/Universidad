Diagrama del circuito.

  diagrama-conexion-rfid.png  - Vista esquematica: UNO R4 WiFi, lector RC522 por SPI,
    LED verde (D7) y LED rojo (D6) con resistencias de 220 ohm, y la matriz
    LED 12x8 integrada.   [LISTO]

Conexion del RC522 (se alimenta con 3.3 V, NUNCA 5 V):
  SDA (SS) -> D10
  SCK      -> D13
  MOSI     -> D11
  MISO     -> D12
  IRQ      -> sin conectar
  GND      -> GND
  RST      -> D9
  3.3V     -> 3.3V

LED verde: D7 -> resistencia -> anodo; catodo -> GND  (acceso concedido)
LED rojo:  D6 -> resistencia -> anodo; catodo -> GND  (acceso denegado)
