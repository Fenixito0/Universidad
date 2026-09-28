Diagrama del circuito.

  diagrama-conexion-rfid.png  - Vista esquematica: Arduino UNO R4 WiFi,
    lector RC522 por SPI, LED de acceso con resistencia de 470 ohm en D7 y la
    matriz LED 12x8 integrada.   [LISTO]

Conexion del RC522 (se alimenta con 3.3 V, NUNCA 5 V):
  SDA (SS) -> D10
  SCK      -> D13
  MOSI     -> D11
  MISO     -> D12
  IRQ      -> sin conectar
  GND      -> GND
  RST      -> D9
  3.3V     -> 3.3V

LED: D7 -> resistencia 470 ohm -> anodo (pata larga); catodo -> GND.
