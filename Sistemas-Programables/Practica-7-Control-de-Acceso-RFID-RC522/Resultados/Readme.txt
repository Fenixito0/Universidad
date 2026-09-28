Resultados de la practica.

Desde el 19-sep-2026 los resultados van escritos directo en la seccion
"Resultados" del Readme.md principal (acuerdo del profesor, "el readme.md es
un glosario, no biblia"). Esta carpeta se deja por consistencia con el
esqueleto del repo.

PENDIENTE: hacer las 4 pruebas de la consigna con el circuito armado y
anotar lo observado en el Readme:
  1. Tarjeta autorizada: el LED verde enciende y se apaga solo a los 2 s.
  2. Tarjeta no autorizada: el verde no enciende (enciende el rojo 2 s).
  3. Con el LED encendido, acercar la no autorizada: debe salir
     "ACCESO DENEGADO" (el programa no se queda bloqueado).
  4. Desconectar MISO (D12) y reiniciar: debe salir el mensaje de error de
     comunicacion; volver a conectarlo.
UIDs ya leidos: llavero 99 EB 7B 63, tarjeta AA 25 60 E4 (ambos autorizados en el codigo).
