# Diseño: prueba del flujo entre agentes

- Rama: `mejora/prueba-flujo` (creada desde `master` @ `42ae7e3`)
- Autor del diseño: Claude (2026-10-09)
- Implementa: el otro agente (o Claude en una tarea aparte). Este documento no modifica código.

## Objetivo

Validar el circuito completo entre agentes (diseño → implementación → versión → revisión) con un cambio de código trivial y sin riesgo: agregar un único comentario de línea en `MiYoke_PD.ino`. Como toca un `.ino`, el cambio debe disparar el versionado automático (`nueva-version.ps1`) y dejar su `ACTUALIZACION_Vx_y.txt`.

## Ubicación exacta

Archivo: `MiYoke_PD.ino` (raíz del proyecto). Estado actual [INO]: 246 líneas, terminadores CRLF, ningún bloque `/* */` en el archivo.

Se inserta **una línea nueva inmediatamente arriba de `void setup() {`**, que hoy está en la línea 98. Después del cambio, el comentario queda en la línea 98 y `void setup() {` pasa a la 99.

Contexto actual [INO] `MiYoke_PD.ino:94-99`:

```cpp
        printPitch = !printPitch;
    }
#endif
}

void setup() {
```

Contexto esperado después del cambio:

```cpp
        printPitch = !printPitch;
    }
#endif
}

// VERSION CON AGENTES CLAUDE Y CODEX
void setup() {
```

Por qué ahí y no en la línea 1:

- Es nivel superior del archivo: la línea 96 (`}`) cierra la función anterior y la línea 95 (`#endif`) cierra el `#if DEBUG_MOTION_TELEMETRY` de la línea 55. Ningún `#if` queda abierto en ese punto, así que el comentario no queda condicionado por ninguna directiva.
- No hay bloques `/* */` en el archivo.
- La línea 1 (`#include "mis_defines.h"`) también sería válida, pero arriba de `setup()` el comentario queda junto al punto de entrada, que es donde se espera leer la nota.

## Texto exacto

```
// VERSION CON AGENTES CLAUDE Y CODEX
```

- Sin sangría (columna 0), un espacio entre `//` y el texto, sin espacios al final.
- Mayúsculas tal cual, sin tildes ni punto final.
- Terminador de línea CRLF, igual que el resto del archivo.

## Criterios de aceptación

1. `git diff master...mejora/prueba-flujo -- MiYoke_PD.ino` muestra **exactamente una línea agregada** (`+// VERSION CON AGENTES CLAUDE Y CODEX`) y **ninguna eliminada ni modificada**. No hay cambios de fin de línea en el resto del archivo (verificar que el diff no reescriba todo el archivo por CRLF/LF).
2. La línea nueva está inmediatamente antes de `void setup() {` y después de la línea en blanco que sigue al `}` de la función anterior.
3. No cambia la lógica: ningún otro archivo de código (`.h .hpp .c .cpp .ino .ps1 .cmd .py .vcxproj`) aparece en el diff. Opcional: el sketch compila igual que en `master`.
4. Se ejecuta `graphify-sync.cmd` tras editar el `.ino` (regla 6 de HANDOFF.md); `graphify-ino/` no se agrega a git.
5. Commit por nombre (`git add MiYoke_PD.ino`), nunca `git add .` ni `git add -A`.
6. Se ejecuta `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Agregar comentario de prueba del flujo entre agentes en MiYoke_PD.ino"`, que imprime `Version Vx.y creada`, y existe `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`. El número de versión lo asigna el script.
7. Se completa "Última entrega" en HANDOFF.md con la versión creada.

## Qué no se toca

- Ninguna otra línea de `MiYoke_PD.ino`: includes, directivas `#if/#endif`, objetos globales, `setup()`, `loop()`.
- Ningún otro archivo de código (`*.h`, `*.cpp`, `SimConnectBridge/`, scripts `.ps1/.cmd`).
- Constantes de seguridad (`MOTOR_PWM_HARD_LIMIT`, `SIMCONNECT_EXTRA_LIMIT`, límites de PWM) ni `mis_defines.h`.
- `graphify-ino/` no se versiona.
- No se integra a `master`: queda en la rama hasta revisión del otro agente y validación del usuario en hardware (regla 4).
