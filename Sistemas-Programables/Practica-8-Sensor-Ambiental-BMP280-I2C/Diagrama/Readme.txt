Diagrama del circuito.

  diagrama-conexion-bme280-i2c.png - Vista esquematica: UNO R4 WiFi (dibujada
    como el UNO de Tinkercad) y sensor BME280/BMP280 por I2C.   [LISTO]

Conexion del sensor (alimentar con 3.3 V):
  VCC -> 3.3V
  GND -> GND
  SDA -> A4 (SDA)
  SCL -> A5 (SCL)
  CSB y SDO sin conectar -> direccion 0x77
  Opcional: CSB -> VCC y SDO -> GND (direccion 0x76)
