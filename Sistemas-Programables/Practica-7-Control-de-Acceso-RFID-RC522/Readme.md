# Nombre del proyecto
Control de acceso con RFID RC522 (bus SPI)

## Descripción
El Arduino se comunica por el bus SPI con un lector RFID RC522, lee el
número único (UID) de una tarjeta o llavero y lo muestra en el Monitor Serie
en hexadecimal. Con ese UID simula un control de acceso: una tarjeta
autorizada enciende el LED verde y una no autorizada el rojo, cada uno
durante 2 segundos. Como extra, la matriz LED integrada de la UNO R4 WiFi
escribe **ACEPTADO** o **RECHAZADO**.

## Objetivos
- Conectar un periférico por el bus SPI (SCK, MOSI, MISO, SS) y verificar que
  hay comunicación antes de usarlo.
- Leer el UID de tarjetas RFID de 13.56 MHz y mostrarlo en hexadecimal.
- Tomar decisiones con el UID (autorizado / no autorizado).
- Controlar el tiempo de los LEDs con `millis()` para que el programa siga
  leyendo tarjetas mientras un LED está encendido.

## Herramientas y material utilizado
- Arduino UNO R4 WiFi (con su matriz LED 12×8 integrada)
- Módulo lector RFID RC522 con tarjeta y llavero
- 1 LED verde y 1 LED rojo con sus resistencias, protoboard y cables
- Librerías `MFRC522` y `ArduinoGraphics` (Library Manager); `SPI` y
  `Arduino_LED_Matrix` vienen con el core de la R4
- Arduino IDE y Monitor Serie

## Diagrama
El RC522 va a los pines SPI de la UNO (SCK D13, MISO D12, MOSI D11, SDA/SS
D10, RST D9) y se alimenta **solo con 3.3 V**: a 5 V se daña. El LED verde va
en D7 y el rojo en D6, cada uno con su resistencia, y todo comparte tierra.

PENDIENTE: diagrama estilo Tinkercad. Ver [Diagrama/Readme.txt](Diagrama/Readme.txt).

## Código
Al arrancar lee el registro de versión del lector para confirmar que hay
comunicación. Cada tarjeta nueva imprime `UID: 3A F2 1C 7B` y se compara con
la lista de autorizadas; cada LED tiene su propio temporizador con `millis()`
y el texto de la matriz avanza una columna cada 60 ms, así que nada bloquea
la lectura. UIDs y librerías en [Codigo/Readme.txt](Codigo/Readme.txt).

[Ver código](Codigo/)

## Reporte
El reporte contiene la metodología (bus SPI, lectura del UID, control de
acceso sin bloqueo y texto en la matriz), las conexiones, el diagrama, las
pruebas y las conclusiones técnicas.

[Ver Reporte](Reporte/Reporte-Control-de-Acceso-RFID.pdf)

## Resultados
PENDIENTE: armar el circuito, anotar el UID de la tarjeta y del llavero y
hacer las 4 pruebas de la consigna (tarjeta autorizada, no autorizada,
no autorizada con el LED encendido y MISO desconectado). Ver
[Resultados/Readme.txt](Resultados/Readme.txt).

## Video
PENDIENTE: [Ver carpeta Video](Video/)

## Conclusiones
El bus SPI permite hablar con el lector usando cuatro líneas compartidas y
una de selección (SS) por dispositivo; comprobar el registro de versión al
arrancar distingue un lector sin comunicación de uno que simplemente no ve
tarjetas.

Usar `millis()` en lugar de `delay()` es lo que permite negar una tarjeta
mientras el LED de otra sigue encendido: el programa nunca se queda
esperando, igual que en la Práctica 2.
