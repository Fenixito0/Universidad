Diagrama del circuito.

PENDIENTE: diagrama estilo Tinkercad hecho con Claude Design (lo sube Jesus).

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
