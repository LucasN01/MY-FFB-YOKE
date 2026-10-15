# Diseño: reorganización en carpetas de los archivos sueltos de la raíz

Autor: Claude (Master Plan Architect), 2026-10-15. Estado: **aprobado por el usuario (2026-10-15)**: estructura recomendada, borrar el PDF duplicado (D4: sí), implementación por Flujo y versión creada por Claude al final.
Relevamiento hecho leyendo los archivos sobre el commit `a79965b` (etiqueta `V2.4`, igual a `master`).

> Aviso de rama: el pedido dice "rama actual: master", pero el árbol de trabajo está en
> `cambio/posicion-simconnect`. Las dos apuntan al mismo commit (`a79965b` = `V2.4`), así que el
> diseño vale igual; la implementación sale de `master` en una rama nueva (paso 1).

---

## 0. Resumen

| Queda en la raíz (proyecto) | Va a `herramientas/` (soporte) | Va / queda en `tesis/` |
|---|---|---|
| Firmware: `MiYoke_PD.ino` + 25 `.h` | `compilacion/`: `probar.ps1`, `Build-Pico2.ps1`, `Build-Bridge.ps1`, `Build-WaitForFlight.ps1` | Sin cambios (ya agrupa todo lo LaTeX) |
| Lanzador: `INICIAR SIMULADOR.cmd`, `INICIAR SIMULADOR V3.cmd`, `Iniciar-Simulador.ps1` | `ensayos/`: `ENSAYO.cmd/.ps1`, `CICLOS.cmd/.ps1`, `ARRANQUE.cmd/.ps1`, `BANCO.ps1`, `FALLA.ps1` | `Efectos_de_simulador_V2_3.pdf` de la raíz se **elimina**: es copia idéntica de `tesis/fundamento_efectos/` |
| Reglas: `CLAUDE.md`, `AGENTS.md`, `HANDOFF.md`, `.git*`, `.graphifyignore` | `flujo/`: `lanzar-codex.py`, `nueva-version.ps1`, `compilar-tesis.ps1`, `graphify-sync.cmd` | |
| Carpetas: `SimConnectBridge/`, `tests/`, `tesis/`, `LEEME md/`, `.claude/` | | |

Plan único para Codex (sin partes), un solo commit atómico de movimientos + rutas + documentación, y una versión nueva (V2.5).

---

## 1. Fundamento (por qué así)

### 1.1 El problema real no es estético: es la "raíz implícita"
Todos los scripts de soporte usan su propia carpeta (`$PSScriptRoot`, `%~dp0`, `__file__`) **como si fuera la raíz del repositorio**. Mientras viven en la raíz, eso es verdad por casualidad. Al moverlos un nivel o dos, cada `Join-Path $PSScriptRoot 'SimConnectBridge\...'` pasa a apuntar a una carpeta que no existe. La reorganización es, en el fondo, hacer **explícita** esa raíz: cada script la calcula una vez (`$Raiz`) y la verifica.

### 1.2 Restricciones duras verificadas (no negociables)
| Restricción | Evidencia | Consecuencia |
|---|---|---|
| Arduino exige que el `.ino` esté en una carpeta con su mismo nombre y las cabeceras junto a él (o en `src/`) | `Build-Pico2.ps1:21` pasa la carpeta del sketch a `arduino-cli` | Firmware queda en la raíz `MiYoke_PD/` |
| El puente compila cabeceras de la raíz | `SimConnectBridge/*.vcxproj:29` `AdditionalIncludeDirectories=$(MSBuildProjectDirectory)\..`; `main.cpp:18`, `Replay.h:11`, `*Model.h:2` incluyen `../X.h` | Firmware en la raíz; `SimConnectBridge/` en la raíz |
| Las pruebas incluyen `../X.h` (18 líneas) | `tests/*.h`, `tests/*.cpp` | `tests/` en la raíz (ver Decisión D2) |
| Acceso directo del escritorio | `OneDrive\Escritorio\INICIAR SIMULADOR V3.lnk` → `MiYoke_PD\INICIAR SIMULADOR V3.cmd` | Lanzador en la raíz |
| Prepar3D V3 apunta a `MiYoke_PD\Prepar3D-v3` | `%APPDATA%\Lockheed Martin\Prepar3D v3\dll.xml:9` y `simobjects.cfg:3` | `Prepar3D-v3/` no se mueve |
| `launcher_port_checks.ps1` busca el lanzador en el padre de `tests/` | `tests/launcher_port_checks.ps1:4` | Lanzador y `tests/` siguen siendo hermanos en la raíz: sin cambios |
| Agentes cargan `CLAUDE.md`, `AGENTS.md` y leen `HANDOFF.md` de la raíz | Reglas del proyecto | Quedan en la raíz |

### 1.3 Precedente
El patrón "scripts en `eng/` o `tools/` que calculan la raíz con ruta fija relativa a sí mismos" es el de dotnet/arcade (`eng/common/tools.ps1`: `$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..\')`). Se adopta lo mismo, con una verificación de existencia para fallar con un mensaje claro si alguien vuelve a mover un script.

---

## 2. Crítica de la propuesta preliminar del usuario

| # | Propuesta | Veredicto | Motivo verificado |
|---|---|---|---|
| C1 | Carpeta `"Herramientas de soporte/"` | **Cambiar** a `herramientas/` | Ver D1. Con espacios, cada comando de `CLAUDE.md`/`AGENTS.md`/skills/LEEME necesita comillas y en Git Bash un error de comillas parte la ruta. `LEEME md/` ya muestra ese costo. |
| C2 | Subcarpeta `ensayos/` dentro de herramientas | **Aceptar, pero corregir `.gitignore`** | `.gitignore:37` dice `ensayos/` sin ancla: ignora **cualquier** carpeta llamada `ensayos`. Verificado: `git check-ignore -v herramientas/ensayos/ENSAYO.ps1` → `.gitignore:37:ensayos/`. Los archivos movidos con `git mv` seguirían en seguimiento, pero cualquier script nuevo ahí quedaría ignorado sin aviso. Se ancla a `/ensayos/`. |
| C3 | `pruebas/` (mover `tests/` adentro) | **Rechazar** | Ver D2: 18 `#include "../"` a cambiar, `.gitignore`/`.graphifyignore`, 6 capítulos de tesis y documentos que citan `tests/...`; y las pruebas son parte del proyecto, no herramienta. |
| C4 | `compilar-tesis.ps1` a `tesis/` | **Rechazar**; va a `herramientas/flujo/` | `CLAUDE.md:47` y `AGENTS.md:17`: "Codex no edita archivos de `tesis/`". El script lo arregla Codex (el último Plan para Codex lo hizo). Ponerlo en `tesis/` choca con la regla. Además el inventario ya lo clasifica como FLUJO. |
| C5 | `Efectos_de_simulador_V2_3.pdf` a `tesis/` | **Cambiar**: eliminarlo de la raíz (`git rm`) | Es copia idéntica de `tesis/fundamento_efectos/Efectos_de_simulador_V2_3.pdf` (mismo blob git `61e32d8c`, mismo SHA-256 `45b0c09c…`). Ningún archivo lo referencia. Moverlo crearía un tercer nombre del mismo binario. Queda en historial y en `tesis/fundamento_efectos/`. |
| C6 | `Iniciar-Simulador.ps1.antes-autocom.bak` a la Papelera | **Aceptar** como paso manual del usuario | No está en git (lo ignora `*.bak`). También existe `LEEME md/LEEME-LANZADOR.md.antes-autocom.bak` (igual situación). Codex no borra archivos fuera de git. |
| C7 | (omitido) `nueva-version.ps1` | **Agregar** dos correcciones críticas | Desde una subcarpeta: (a) `Split-Path $PSScriptRoot -Parent` crearía `MiYoke_PD_VERSIONES` **dentro del repo**; (b) `git archive HEAD` y `git ls-files` desde una subcarpeta solo toman esa subcarpeta: la copia de versión quedaría incompleta y el control `$copiados -lt $esperados` no lo detectaría (ambos números se achican). Se fija `Set-Location $Raiz`. |
| C8 | (omitido) `graphify-sync.cmd` | **Agregar** | Hoy usa rutas relativas al directorio actual (no a sí mismo): solo funciona si se ejecuta desde la raíz. Movido, falla siempre. Se usa `pushd "%~dp0..\.."`. |
| C9 | (omitido) regex `$flujo` | **Agregar** | `nueva-version.ps1:19` excluye por ruta exacta de la raíz. Con las rutas nuevas, las herramientas del flujo volverían a generar versión. Se actualiza manteniendo **lista explícita** (no la carpeta entera), para respetar la regla "preguntar antes de excluir un script nuevo". |
| C10 | (omitido) codificación | **Agregar** | `BANCO`, `Build-Bridge`, `CICLOS`, `ENSAYO`, `Iniciar-Simulador` tienen BOM UTF-8 y `CICLOS`/`ENSAYO` tienen texto no ASCII: PowerShell 5.1 sin BOM los leería como ANSI. Las ediciones deben conservar BOM y CRLF. |
| C11 | (omitido) árbol sucio | **Agregar precondición** | `nueva-version.ps1:6` exige árbol limpio salvo `graphify-ino/`. Hoy hay `M tesis/main.tex` y dos `.tex` sin seguimiento del usuario: el script se detendría. Ver P3. |

---

## 3. Estructura final

```text
MiYoke_PD/                          <- carpeta del sketch Arduino (el nombre no cambia)
├─ .claude/  .gitattributes  .gitignore  .graphifyignore
├─ AGENTS.md  CLAUDE.md  HANDOFF.md
├─ MiYoke_PD.ino  mis_defines.h  + 24 cabeceras .h del firmware (sin cambios)
├─ INICIAR SIMULADOR.cmd  INICIAR SIMULADOR V3.cmd  Iniciar-Simulador.ps1   (sin cambios)
├─ SimConnectBridge/                (sin cambios)
├─ tests/                           (sin cambios)
├─ tesis/                           (sin cambios; Efectos_..._V2_3.pdf ya está en fundamento_efectos/)
├─ LEEME md/                        (solo ediciones de rutas, sección 9)
├─ herramientas/
│  ├─ compilacion/  probar.ps1  Build-Pico2.ps1  Build-Bridge.ps1  Build-WaitForFlight.ps1
│  ├─ ensayos/      ENSAYO.cmd  ENSAYO.ps1  CICLOS.cmd  CICLOS.ps1  ARRANQUE.cmd  ARRANQUE.ps1  BANCO.ps1  FALLA.ps1
│  └─ flujo/        lanzar-codex.py  nueva-version.ps1  compilar-tesis.ps1  graphify-sync.cmd
│
│  (fuera de git, no se mueven)
├─ Prepar3D-v3/  VPforce-TelemFFB-refactor/  .codex/
└─ build-pico2*/  ensayos/  logs/  tmp/  graphify-out/  graphify-ino/   <- generadas, siguen en la raíz
```

Notas:
- `ensayos/` de la raíz (resultados, ignorada) y `herramientas/ensayos/` (scripts, en git) son cosas distintas; los scripts siguen escribiendo en `ensayos/` de la raíz, como hoy.
- `build-pico2*/` sigue en la raíz: `ARRANQUE.ps1` y `LEEME md/ENSAYOS.md` la buscan ahí.

---

## 4. Tabla origen → destino

Todos con `git mv` (preserva historial; `git log --follow` sigue funcionando).

| # | Origen | Destino | Acción |
|---|---|---|---|
| 1 | `probar.ps1` | `herramientas/compilacion/probar.ps1` | `git mv` + editar |
| 2 | `Build-Pico2.ps1` | `herramientas/compilacion/Build-Pico2.ps1` | `git mv` + editar |
| 3 | `Build-Bridge.ps1` | `herramientas/compilacion/Build-Bridge.ps1` | `git mv` + editar |
| 4 | `Build-WaitForFlight.ps1` | `herramientas/compilacion/Build-WaitForFlight.ps1` | `git mv` + editar |
| 5 | `ENSAYO.cmd` | `herramientas/ensayos/ENSAYO.cmd` | `git mv` (sin edición: `%~dp0ENSAYO.ps1` sigue al lado) |
| 6 | `ENSAYO.ps1` | `herramientas/ensayos/ENSAYO.ps1` | `git mv` + editar |
| 7 | `CICLOS.cmd` | `herramientas/ensayos/CICLOS.cmd` | `git mv` (sin edición) |
| 8 | `CICLOS.ps1` | `herramientas/ensayos/CICLOS.ps1` | `git mv` + editar |
| 9 | `ARRANQUE.cmd` | `herramientas/ensayos/ARRANQUE.cmd` | `git mv` + editar |
| 10 | `ARRANQUE.ps1` | `herramientas/ensayos/ARRANQUE.ps1` | `git mv` + editar |
| 11 | `BANCO.ps1` | `herramientas/ensayos/BANCO.ps1` | `git mv` + editar |
| 12 | `FALLA.ps1` | `herramientas/ensayos/FALLA.ps1` | `git mv` (sin edición: no usa rutas del repo) |
| 13 | `lanzar-codex.py` | `herramientas/flujo/lanzar-codex.py` | `git mv` + editar |
| 14 | `nueva-version.ps1` | `herramientas/flujo/nueva-version.ps1` | `git mv` + editar |
| 15 | `compilar-tesis.ps1` | `herramientas/flujo/compilar-tesis.ps1` | `git mv` + editar |
| 16 | `graphify-sync.cmd` | `herramientas/flujo/graphify-sync.cmd` | `git mv` + reescribir (3 → 5 líneas) |
| 17 | `Efectos_de_simulador_V2_3.pdf` | — | `git rm` (duplicado; ver C5) |

Comandos exactos (desde la raíz, PowerShell):

```powershell
New-Item -ItemType Directory -Force herramientas\compilacion, herramientas\ensayos, herramientas\flujo | Out-Null
git mv probar.ps1 Build-Pico2.ps1 Build-Bridge.ps1 Build-WaitForFlight.ps1 herramientas/compilacion/
git mv ENSAYO.cmd ENSAYO.ps1 CICLOS.cmd CICLOS.ps1 ARRANQUE.cmd ARRANQUE.ps1 BANCO.ps1 FALLA.ps1 herramientas/ensayos/
git mv lanzar-codex.py nueva-version.ps1 compilar-tesis.ps1 graphify-sync.cmd herramientas/flujo/
git rm Efectos_de_simulador_V2_3.pdf
```

---

## 5. Cambios exactos en scripts

Reglas para Codex en esta sección:
- Conservar codificación y finales de línea de cada archivo (BOM UTF-8 donde lo hay; CRLF). Verificación en 7.3.
- "Bloque RAIZ" = estas dos líneas exactas (PowerShell):

```powershell
$Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'MiYoke_PD.ino'))) { throw "No se encontro MiYoke_PD.ino en $Raiz (raiz del repositorio)." }
```

Los números de línea son los del archivo **antes** de editar.

### 5.1 `herramientas/compilacion/probar.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| 5 | `# Uso: powershell -ExecutionPolicy Bypass -File probar.ps1 [-Pico] [-Encoders] [-Potentiometers] [-Bridge] [-PruebaFalla] [-Todo]` | `# Uso: powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 [-Pico] [-Encoders] [-Potentiometers] [-Bridge] [-PruebaFalla] [-Todo]` |
| 25 | `Set-Location $PSScriptRoot` | Bloque RAIZ seguido de `Set-Location $Raiz` (3 líneas) |
| 81, 85, 89, 97 | `-File 'Build-Pico2.ps1'` (4 apariciones) | `-File 'herramientas\compilacion\Build-Pico2.ps1'` |
| 93 | `-File 'Build-Bridge.ps1'` | `-File 'herramientas\compilacion\Build-Bridge.ps1'` |

Las rutas `tests\...` (líneas 62, 66, 69, 74) no cambian: son relativas a `$Raiz` por el `Set-Location`.

### 5.2 `herramientas/compilacion/Build-Pico2.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 7 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 17 | `    '--output-dir', (Join-Path $PSScriptRoot $outputFolder))` | `    '--output-dir', (Join-Path $Raiz $outputFolder))` |
| 21 | `$buildArgs += $PSScriptRoot` | `$buildArgs += $Raiz` |

Crítico: la línea 21 es la carpeta del sketch; si queda `$PSScriptRoot`, `arduino-cli` busca `compilacion.ino` y falla.

### 5.3 `herramientas/compilacion/Build-Bridge.ps1` (tiene BOM)
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 1 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 7 | `& $builder (Join-Path $PSScriptRoot 'SimConnectBridge\YokeSimConnectBridge.vcxproj')` | `& $builder (Join-Path $Raiz 'SimConnectBridge\YokeSimConnectBridge.vcxproj')` (resto de la línea igual) |

### 5.4 `herramientas/compilacion/Build-WaitForFlight.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 1 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 7 | `& $builder (Join-Path $PSScriptRoot 'SimConnectBridge\WaitForFlight.vcxproj')` | `& $builder (Join-Path $Raiz 'SimConnectBridge\WaitForFlight.vcxproj')` (resto igual) |

### 5.5 `herramientas/ensayos/ENSAYO.ps1` (BOM, no ASCII)
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 5 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 6 | `$destino = Join-Path $PSScriptRoot ('ensayos\' + ` | `$destino = Join-Path $Raiz ('ensayos\' + ` (resto igual) |
| 61 | `$registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'` | `$registros = Join-Path $Raiz 'SimConnectBridge\bin\Win32\RealismoSC8'` |
| 71 | `& (Join-Path $PSScriptRoot 'tests\bin\comparar_ensayo.exe') $destino \| Out-Null` | `& (Join-Path $Raiz 'tests\bin\comparar_ensayo.exe') $destino \| Out-Null` |

### 5.6 `herramientas/ensayos/CICLOS.ps1` (BOM, no ASCII)
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 1 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 2 | `$destino = Join-Path $PSScriptRoot ('ensayos\' + ` | `$destino = Join-Path $Raiz ('ensayos\' + ` (resto igual) |
| 37 | `$registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'` | `$registros = Join-Path $Raiz 'SimConnectBridge\bin\Win32\RealismoSC8'` |

La línea 14 (texto de la rutina: `-File Iniciar-Simulador.ps1 -DetectarPlaca`) no cambia: el lanzador sigue en la raíz.

### 5.7 `herramientas/ensayos/ARRANQUE.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 5 | (`)` que cierra `param`) | insertar Bloque RAIZ (antes de `function Select-Elf`) |
| 8 | `    Join-Path $PSScriptRoot ($build + '\MiYoke_PD.ino.elf')` | `    Join-Path $Raiz ($build + '\MiYoke_PD.ino.elf')` |
| 20 | `-File (Join-Path $PSScriptRoot 'Iniciar-Simulador.ps1') -DetectarPlaca` | `-File (Join-Path $Raiz 'Iniciar-Simulador.ps1') -DetectarPlaca` |
| 32 | `    $registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'` | `    $registros = Join-Path $Raiz 'SimConnectBridge\bin\Win32\RealismoSC8'` |
| 35 | `    $destino = Join-Path $PSScriptRoot ('ensayos\' + ` | `    $destino = Join-Path $Raiz ('ensayos\' + ` (resto igual) |

Líneas 76 y 81 (`Join-Path $PSScriptRoot 'FALLA.ps1'`) **no cambian**: `FALLA.ps1` está en la misma carpeta.

### 5.8 `herramientas/ensayos/BANCO.ps1` (BOM)
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| después de 7 | (`$ErrorActionPreference = 'Stop'`) | insertar Bloque RAIZ |
| 22 | `-File (Join-Path $PSScriptRoot 'Iniciar-Simulador.ps1') -DetectarPlaca` | `-File (Join-Path $Raiz 'Iniciar-Simulador.ps1') -DetectarPlaca` |
| 28 | `$destino = Join-Path $PSScriptRoot ('ensayos\' + ` | `$destino = Join-Path $Raiz ('ensayos\' + ` (resto igual) |
| 32 | `$registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'` | `$registros = Join-Path $Raiz 'SimConnectBridge\bin\Win32\RealismoSC8'` |
| 46 | `& (Join-Path $PSScriptRoot 'tests\bin\comparar_ensayo.exe') $destino \| Out-Null` | `& (Join-Path $Raiz 'tests\bin\comparar_ensayo.exe') $destino \| Out-Null` |

`-Entrada` sigue siendo relativa al directorio actual del usuario (comportamiento actual, sin cambio).

### 5.9 `herramientas/ensayos/ARRANQUE.cmd`
Dos apariciones (líneas 4 y 7) de `"%~dp0tmp\vuelo_corto\telemetry.csv"` → `"%~dp0..\..\tmp\vuelo_corto\telemetry.csv"`.
`"%~dp0ARRANQUE.ps1"` (línea 7) no cambia. El texto del `echo` (línea 5) no cambia.

### 5.10 `herramientas/flujo/nueva-version.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| 3 | `Set-Location $PSScriptRoot` | Bloque RAIZ seguido de `Set-Location $Raiz` (3 líneas) |
| 19 | `  $flujo = '^(lanzar-codex\.py\|compilar-tesis\.ps1\|nueva-version\.ps1\|graphify-sync\.cmd\|tesis/.*)$'` | `  $flujo = '^(herramientas/flujo/(lanzar-codex\.py\|compilar-tesis\.ps1\|nueva-version\.ps1\|graphify-sync\.cmd)\|tesis/.*)$'` |
| 27 | `$dest = Join-Path (Split-Path $PSScriptRoot -Parent) "MiYoke_PD_VERSIONES\$carpeta"` | `$dest = Join-Path (Split-Path $Raiz -Parent) "MiYoke_PD_VERSIONES\$carpeta"` |

(En la tabla, `\|` es el `|` literal del archivo, escapado por Markdown.)
El comentario de la línea 18 queda igual. La lista sigue siendo explícita por archivo (regla de `CLAUDE.md:36`).

### 5.11 `herramientas/flujo/compilar-tesis.ps1`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| 8 | `# Uso: powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1 [-Archivo tesis\main.tex] [-MaxErrores 15] [-CopiarPdf]` | `# Uso: powershell -ExecutionPolicy Bypass -File herramientas/flujo/compilar-tesis.ps1 [-Archivo tesis\main.tex] [-MaxErrores 15] [-CopiarPdf]` |
| 17 | `$raiz = Split-Path -Parent $MyInvocation.MyCommand.Path` | `$raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path` |
| después de 17 | — | `if (-not (Test-Path -LiteralPath (Join-Path $raiz 'MiYoke_PD.ino'))) { Write-Output "ERROR: no se encontro la raiz del repositorio ($raiz)"; exit 2 }` |

`$raiz` sigue siendo la raíz del repo: `-Archivo tesis\main.tex`, `tmp\tesis-build` y `-CopiarPdf` se comportan igual que hoy. (Se usa `Write-Output`/`exit 2` en lugar de `throw` para respetar el formato de una línea del script.)

### 5.12 `herramientas/flujo/lanzar-codex.py`
| Línea | Texto actual | Texto nuevo |
|---|---|---|
| 9 | `  python lanzar-codex.py "<mensaje>" [-o tmp/codex_ultimo.txt] [--add-dir DIR ...] [--titulo T]` | `  python herramientas/flujo/lanzar-codex.py "<mensaje>" [-o tmp/codex_ultimo.txt] [--add-dir DIR ...] [--titulo T]` |
| 22 | `REPO = os.path.dirname(os.path.abspath(__file__))` | las 4 líneas siguientes |

```python
# El script vive en herramientas/flujo/: la raiz del repo esta dos niveles arriba.
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if not os.path.isfile(os.path.join(REPO, "MiYoke_PD.ino")):
    sys.exit(f"[lanzar-codex] no se encontro MiYoke_PD.ino en {REPO} (raiz del repositorio)")
```

`REPO` es también el `cwd` del chat de Codex (línea 118 actual): debe seguir siendo la raíz para que el chat aparezca en el proyecto MY FFB YOKE de la app.

### 5.13 `herramientas/flujo/graphify-sync.cmd` (reescritura completa, CRLF)

```bat
@echo off
pushd "%~dp0..\.."
copy /y MiYoke_PD.ino graphify-ino\MiYoke_PD_ino.cpp >nul
graphify update .
popd
```

### 5.14 Sin cambios (verificado)
`ENSAYO.cmd`, `CICLOS.cmd`, `FALLA.ps1`, `INICIAR SIMULADOR.cmd`, `INICIAR SIMULADOR V3.cmd`, `Iniciar-Simulador.ps1` (usa `$PSScriptRoot` = raíz, sigue en la raíz), `tests/*` (incluido `launcher_port_checks.ps1`), `SimConnectBridge/*`, firmware, `.gitattributes`, `.graphifyignore` (sus entradas `tesis/`, `tests/` siguen válidas), `.claude/settings.json`, `.codex/hooks.json`, ganchos de git (`post-commit`/`post-checkout` de graphify no dependen de rutas de scripts).

---

## 6. `.gitignore`

| Línea | Texto actual | Texto nuevo |
|---|---|---|
| 37 | `ensayos/` | `/ensayos/` |

Nada más. (El archivo tiene CRLF en las líneas 1-37 y LF en la última: conservar.)

---

## 7. Orden de pasos (Codex)

### 7.1 Precondiciones
- **P1.** `git status --porcelain`: lo único permitido es `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex`, `tesis/caracterizacion_motores_775.tex` (del usuario: **no tocarlos ni commitearlos**). Si hay otra cosa, `BLOQUEO:`.
- **P2.** `git describe --tags` en `master` = `V2.4`.
- **P3.** Codex **no** ejecuta `nueva-version.ps1` (decisión del usuario): la versión V2.5 la crea Claude después de la revisión, apartando temporalmente los cambios de tesis del usuario. Omitir el paso 9.
- **P4.** Línea base: antes de mover nada, ejecutar `powershell -ExecutionPolicy Bypass -File probar.ps1 -Todo` y `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1`; copiar sus líneas OK/FALLO en HANDOFF ("Línea base"). Sirven para comparar en 8.

### 7.2 Pasos
1. Verificar que la rama actual es `cambio/reorganizar-carpetas` (ya creada por Claude desde `master`, con este diseño commiteado). No crear otra.
2. Ejecutar P4 (línea base).
3. Movimientos y borrado de la sección 4.
4. Ediciones de scripts de la sección 5.
5. `.gitignore` (sección 6).
6. Documentación de la sección 9 (todas las filas marcadas "Codex").
7. Pruebas de la sección 8.1 a 8.6. Si alguna falla, depurar (skill `depuracion-sistematica`) sin salirse del alcance.
8. Un solo commit, por nombre (rutas viejas y nuevas; `git mv`/`git rm` ya dejaron las rutas viejas en el índice):
   ```powershell
   git add -- herramientas/compilacion/probar.ps1 herramientas/compilacion/Build-Pico2.ps1 herramientas/compilacion/Build-Bridge.ps1 herramientas/compilacion/Build-WaitForFlight.ps1 herramientas/ensayos/ENSAYO.cmd herramientas/ensayos/ENSAYO.ps1 herramientas/ensayos/CICLOS.cmd herramientas/ensayos/CICLOS.ps1 herramientas/ensayos/ARRANQUE.cmd herramientas/ensayos/ARRANQUE.ps1 herramientas/ensayos/BANCO.ps1 herramientas/ensayos/FALLA.ps1 herramientas/flujo/lanzar-codex.py herramientas/flujo/nueva-version.ps1 herramientas/flujo/compilar-tesis.ps1 herramientas/flujo/graphify-sync.cmd .gitignore CLAUDE.md AGENTS.md HANDOFF.md .claude/skills/verificacion-antes-de-terminar/SKILL.md "LEEME md/ENSAYOS.md" "LEEME md/LEEME_PICO2.md" "LEEME md/LEEME_SIMCONNECT.md" "LEEME md/INVENTARIO_CODIGO.md"
   git commit -m "Reorganiza scripts de soporte en herramientas/ (compilacion, ensayos, flujo) y elimina PDF duplicado de la raiz"
   ```
   Verificar con `git show --stat HEAD` que aparecen 16 renombres (`R`), 1 borrado (`D` del PDF) y ningún archivo de `tesis/`.
9. (Omitido: la versión la crea Claude después de la revisión.)
10. `herramientas\flujo\graphify-sync.cmd` (actualiza el grafo con las rutas nuevas).
11. HANDOFF, "Última entrega": commits, líneas OK/FALLO (base y final) y la lista de 8.7 para el usuario. Indicar "versión: pendiente, la crea Claude".

### 7.3 Por qué un solo plan y un solo commit
Es mecánico (sin lógica nueva) y el riesgo principal es la **incoherencia transitoria**: si los movimientos se commitean en una parte y la documentación en otra, entre ambas `AGENTS.md` ordenaría `-File probar.ps1` (que ya no existe) y Claude lanzaría `lanzar-codex.py` desde una ruta que ya no existe. Un commit atómico garantiza que cualquier `git checkout` quede consistente. No se divide en partes.

Nota de ejecución: Codex se lanza con la ruta **vieja** (`python lanzar-codex.py ...`); mover el `.py` durante la sesión es seguro (CPython cierra el archivo antes de ejecutar y `REPO` ya está en memoria). Los lanzamientos siguientes usan la ruta nueva.

---

## 8. Pruebas y criterios de aceptación

### 8.1 Suite completa (obligatoria)
```powershell
powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 -Todo
```
Aceptación: todas las líneas `OK`, las mismas que la línea base (P4). Además:
```powershell
Get-Item build-pico2\MiYoke_PD.ino.uf2, build-pico2-encoders\MiYoke_PD.ino.uf2, build-pico2-potentiometers\MiYoke_PD.ino.uf2 | Select-Object FullName, LastWriteTime
Test-Path herramientas\compilacion\build-pico2   # debe dar False
```
Los `.uf2` deben estar en la raíz con fecha de esta corrida.

### 8.2 Ejecución desde otra carpeta (prueba de la raíz explícita)
```powershell
Push-Location $env:TEMP; powershell -ExecutionPolicy Bypass -File "<repo>\herramientas\compilacion\probar.ps1"; Pop-Location
```
Aceptación: mismas líneas `OK` de la suite y del lanzador.

### 8.3 Sin `$PSScriptRoot` como raíz
```powershell
git grep -n "PSScriptRoot" -- herramientas
```
Aceptación: exactamente estas apariciones: una por cada Bloque RAIZ (`probar`, `Build-Pico2`, `Build-Bridge`, `Build-WaitForFlight`, `ENSAYO`, `CICLOS`, `ARRANQUE`, `BANCO`, `nueva-version`), la línea 17 nueva de `compilar-tesis.ps1`, y las dos de `FALLA.ps1` en `ARRANQUE.ps1`. Ninguna otra.

### 8.4 Sintaxis y codificación
```powershell
Get-ChildItem herramientas -Recurse -Filter *.ps1 | ForEach-Object { $e=$null; [void][Management.Automation.Language.Parser]::ParseFile($_.FullName,[ref]$null,[ref]$e); "$($_.Name) errores=$($e.Count) bom=$(([IO.File]::ReadAllBytes($_.FullName)[0..2] -join ',') -eq '239,187,191')" }
```
Aceptación: `errores=0` en todos; `bom=True` en `BANCO`, `Build-Bridge`, `CICLOS`, `ENSAYO`; `bom=False` en el resto (igual que hoy).

### 8.5 Herramientas del flujo
```powershell
powershell -ExecutionPolicy Bypass -File herramientas/flujo/compilar-tesis.ps1
python -c "import importlib.util as u; s=u.spec_from_file_location('l','herramientas/flujo/lanzar-codex.py'); m=u.module_from_spec(s); s.loader.exec_module(m); print(m.REPO); print(m.titulo_desde_handoff('parte 1'))"
```
Aceptación: `compilar-tesis` da la misma línea que la base (OK o el mismo FALLO por los `.tex` en curso del usuario) y el PDF queda en `tmp\tesis-build` de la raíz; el `python` imprime la ruta de la raíz (termina en `\MiYoke_PD`) y un título (no `None`) o `None` si el Plan para Codex no tiene línea `Tarea:`.
`graphify-sync.cmd`: ejecutarlo desde `%TEMP%` (`cmd /c "cd /d %TEMP% && "<repo>\herramientas\flujo\graphify-sync.cmd""`) y comprobar que cambió la fecha de `graphify-ino\MiYoke_PD_ino.cpp`.

### 8.6 Scripts de ensayo sin hardware
```powershell
powershell -ExecutionPolicy Bypass -File herramientas/ensayos/ARRANQUE.ps1 -Entrada noexiste.csv
```
Aceptación: `Error: No existe la entrada: noexiste.csv` y código 1 (prueba que el Bloque RAIZ pasa y el script parsea). `ENSAYO`, `CICLOS` y `BANCO` no se ejecutan (piden consola o hardware): quedan cubiertos por 8.3 y 8.4 y por 8.7.

### 8.7 Verificación manual del usuario (próxima sesión con el yugo)
1. Doble clic en el acceso directo del escritorio `INICIAR SIMULADOR V3`: debe iniciar igual que antes (no se tocó).
2. Doble clic en `herramientas\ensayos\ENSAYO.cmd`: debe crear `MiYoke_PD\ensayos\<fecha>_vuelo1\rutina.txt` **en la raíz**. Si es solo una prueba, cerrar la ventana y borrar esa carpeta.
3. Primer `ARRANQUE.cmd`/`BANCO.ps1` real: los registros aparecen en `ensayos\` de la raíz.

### 8.8 Criterios de aceptación globales
- `git ls-files | Where-Object { $_ -notmatch '/' }` lista exactamente 35 archivos: 6 de reglas/configuración (`.gitattributes`, `.gitignore`, `.graphifyignore`, `AGENTS.md`, `CLAUDE.md`, `HANDOFF.md`), 26 de firmware (`MiYoke_PD.ino` + 25 `.h`) y 3 del lanzador.
- 8.1 a 8.6 en OK. Versión V2.5 creada fuera del repo.
- `git check-ignore -v herramientas/ensayos/ENSAYO.ps1` no devuelve nada; `git check-ignore -v ensayos/x` sigue devolviendo `/ensayos/`.
- `git log --follow --oneline herramientas/compilacion/probar.ps1` muestra historia anterior a la mudanza.
- Ningún archivo de `tesis/` en el diff; ningún `DISENO_*.md` modificado.

---

## 9. Documentación afectada

Criterio (regla DOCUMENTACIÓN RELACIONADA): se corrige solo lo que queda **incorrecto**. Un comando ejecutable con ruta de la raíz (`-File probar.ps1`, `./Build-Pico2.ps1`) queda incorrecto y se cambia. Una mención en prosa por nombre (`Build-Pico2.ps1 -Encoders fuerza el modo 0`) sigue siendo correcta y **no** se toca. Los `DISENO_*.md`, `VALIDACION_*`, `CAMBIOS_*` y el historial de HANDOFF son registros de su fecha: no se tocan.

**Tesis: no afecta.** Los capítulos citan `tests/...` (no se mueve) y `Build-Pico2.ps1` solo por nombre (`03_centrado.tex:44`, `09_damper.tex:70`). No hay que preguntar al usuario.

Reemplazos exactos de fragmentos (todas las apariciones salvo que se indique; "n" = cantidad esperada, verificada):

### 9.1 `CLAUDE.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | `(ejecutar graphify-sync.cmd tras editarlo)` | `(ejecutar herramientas/flujo/graphify-sync.cmd tras editarlo)` |
| 1 | `Verificar con compilar-tesis.ps1 (salida` | `Verificar con herramientas/flujo/compilar-tesis.ps1 (salida` |
| 1 | `-File probar.ps1` | `-File herramientas/compilacion/probar.ps1` |
| 1 | `ejecutar nueva-version.ps1 siempre desde PowerShell` | `ejecutar herramientas/flujo/nueva-version.ps1 siempre desde PowerShell` |
| 1 | `-File nueva-version.ps1` | `-File herramientas/flujo/nueva-version.ps1` |
| 1 | `(lanzar-codex.py, compilar-tesis.ps1, nueva-version.ps1, graphify-sync.cmd)` | `(carpeta herramientas/flujo/: lanzar-codex.py, compilar-tesis.ps1, nueva-version.ps1, graphify-sync.cmd)` |
| 2 | `-File compilar-tesis.ps1` (línea 46, dos veces) | `-File herramientas/flujo/compilar-tesis.ps1` |
| 1 | `python lanzar-codex.py` | `python herramientas/flujo/lanzar-codex.py` |

Y agregar, inmediatamente antes de `## AHORRO DE TOKENS: graphify, PDF y LaTeX (regla del usuario)`, esta sección (más una línea en blanco):

```markdown
## ESTRUCTURA DE CARPETAS
- Raiz: solo el proyecto. Firmware (MiYoke_PD.ino y sus .h; Arduino exige que esten juntos en la carpeta MiYoke_PD y el puente y tests/ los incluyen como ../X.h), lanzador (INICIAR SIMULADOR*.cmd e Iniciar-Simulador.ps1; los usa el acceso directo del escritorio) y reglas de agentes (CLAUDE.md, AGENTS.md, HANDOFF.md).
- SimConnectBridge/ (puente), tests/ (pruebas C++ y del lanzador), tesis/ (LaTeX), LEEME md/ (documentacion).
- herramientas/compilacion/: probar.ps1, Build-Pico2.ps1, Build-Bridge.ps1, Build-WaitForFlight.ps1.
- herramientas/ensayos/: ENSAYO, CICLOS y ARRANQUE (.cmd y .ps1), BANCO.ps1, FALLA.ps1. Guardan resultados en ensayos/ de la raiz.
- herramientas/flujo/: lanzar-codex.py, nueva-version.ps1, compilar-tesis.ps1, graphify-sync.cmd (no generan version).
- Los scripts de herramientas/ calculan la raiz como dos niveles arriba de su carpeta y funcionan desde cualquier directorio. Los comandos de estas reglas se escriben desde la raiz y con / (sirven en PowerShell y en Git Bash).
- Generadas e ignoradas, en la raiz: build-pico2*/, ensayos/, logs/, tmp/, graphify-out/, graphify-ino/. Fuera de git: Prepar3D-v3/ (la configuracion de Prepar3D V3 apunta ahi: no mover) y VPforce-TelemFFB-refactor/ (referencia).
```

### 9.2 `AGENTS.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | `-File probar.ps1` | `-File herramientas/compilacion/probar.ps1` |
| 1 | `ejecutar nueva-version.ps1 siempre desde PowerShell` | `ejecutar herramientas/flujo/nueva-version.ps1 siempre desde PowerShell` |
| 1 | `(ejecutar graphify-sync.cmd tras editarlo)` | `(ejecutar herramientas/flujo/graphify-sync.cmd tras editarlo)` |
| 1 | `-File nueva-version.ps1` | `-File herramientas/flujo/nueva-version.ps1` |
| 1 | `(lanzar-codex.py, compilar-tesis.ps1, nueva-version.ps1, graphify-sync.cmd)` | `(carpeta herramientas/flujo/: lanzar-codex.py, compilar-tesis.ps1, nueva-version.ps1, graphify-sync.cmd)` |

Y agregar la misma sección `## ESTRUCTURA DE CARPETAS` de 9.1 inmediatamente antes de `## AHORRO DE TOKENS: graphify, PDF y LaTeX (regla del usuario)`.

### 9.3 `HANDOFF.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | ``ejecutar `graphify-sync.cmd`.`` (regla 6) | ``ejecutar `herramientas/flujo/graphify-sync.cmd`.`` |

La sección "Plan para Codex" de esta tarea la escribe Claude ya con las rutas nuevas; Codex no corrige rutas en planes anteriores.

### 9.4 `.claude/skills/verificacion-antes-de-terminar/SKILL.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | `-File probar.ps1` | `-File herramientas/compilacion/probar.ps1` |
| 1 | `-File compilar-tesis.ps1` | `-File herramientas/flujo/compilar-tesis.ps1` |

Las demás menciones de `probar.ps1` en skills (`depuracion-sistematica` y `recibir-revision`) son por nombre: no se tocan.

### 9.5 `LEEME md/ENSAYOS.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | ``Abrir `ENSAYO.cmd` o ejecutar:`` | ``Abrir `herramientas\ensayos\ENSAYO.cmd` o ejecutar desde la raíz del repositorio:`` |
| 1 | `-File ENSAYO.ps1 -Nombre vuelo1 -Rutina corta` | `-File herramientas/ensayos/ENSAYO.ps1 -Nombre vuelo1 -Rutina corta` |
| 1 | `-File BANCO.ps1 -Entrada tmp\vuelo_corto\telemetry.csv` | `-File herramientas/ensayos/BANCO.ps1 -Entrada tmp\vuelo_corto\telemetry.csv` |

Y agregar después de la línea 1 (`# Ensayos del Yoke SC8`) una línea en blanco y:
`Los scripts de ensayo (ENSAYO, CICLOS, ARRANQUE, BANCO, FALLA) están en herramientas/ensayos/; los resultados quedan en ensayos/ de la raíz.`

### 9.6 `LEEME md/LEEME_PICO2.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | `Tambien se puede compilar desde PowerShell, situado en esta carpeta:` | `Tambien se puede compilar desde PowerShell, situado en la raiz del repositorio:` |
| 2 | `.\Build-Pico2.ps1` (líneas 39 y 40, al inicio de línea) | `.\herramientas\compilacion\Build-Pico2.ps1` |
| 1 | `probar.ps1 -PruebaFalla compila` | `herramientas/compilacion/probar.ps1 -PruebaFalla compila` |

### 9.7 `LEEME md/LEEME_SIMCONNECT.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 3 | `./Build-Pico2.ps1` (línea 23) | `./herramientas/compilacion/Build-Pico2.ps1` |

### 9.8 `LEEME md/INVENTARIO_CODIGO.md` (Codex)
| n | Texto actual | Texto nuevo |
|---|---|---|
| 1 | `## 4. Scripts de uso del yugo (raíz)` | ``## 4. Scripts de uso del yugo (lanzador en la raíz; ensayos en `herramientas/ensayos/`)`` |
| 1 | `## 5. Compilación y verificación` | ``## 5. Compilación y verificación (`herramientas/compilacion/`)`` |
| 1 | `## 6. Herramientas del flujo Claude–Codex (no generan versión)` | ``## 6. Herramientas del flujo Claude–Codex (`herramientas/flujo/`, no generan versión)`` |

Y agregar al final de "## Resumen" (después de la última línea):
``Reorganización del 2026-10-15 (`DISENO_reorganizacion_carpetas.md`): los scripts de soporte pasaron a `herramientas/` (`compilacion/`, `ensayos/`, `flujo/`) y se eliminó de la raíz la copia duplicada de `Efectos_de_simulador_V2_3.pdf` (queda en `tesis/fundamento_efectos/`).``

### 9.9 Fuera del repo (no lo hace Codex)
- Memoria de Claude (`flujo-entre-agentes.md`): el comando de lanzamiento pasa a `python herramientas/flujo/lanzar-codex.py`. La actualiza el usuario (según su pedido).
- Sin cambios: acceso directo del escritorio, configuración de Prepar3D V3, `~/.codex/config.toml` (el proyecto sigue siendo la raíz).

---

## 10. Riesgos y mitigaciones

| # | Riesgo | Probabilidad / impacto | Mitigación |
|---|---|---|---|
| R1 | `nueva-version.ps1` desde la subcarpeta crea `MiYoke_PD_VERSIONES` dentro del repo y archiva solo `herramientas/flujo/` | Alta / alto (versiones corruptas sin aviso) | 5.10 (`Set-Location $Raiz`, `Split-Path $Raiz`); aceptación en paso 9 |
| R2 | `.gitignore` `ensayos/` ignora `herramientas/ensayos/` | Cierta / medio (scripts futuros sin seguimiento) | Sección 6; aceptación 8.8 |
| R3 | `Build-Pico2.ps1` pasa la carpeta equivocada como sketch | Cierta si se omite / alto | 5.2 línea 21; 8.1 verifica `.uf2` en la raíz |
| R4 | Pérdida de BOM al editar → texto no ASCII roto en PowerShell 5.1 | Media / medio | Regla de 5; prueba 8.4 |
| R5 | Árbol sucio por la tesis del usuario bloquea la versión | Cierta hoy / bajo | P3: BLOQUEO controlado; mejor commitear antes |
| R6 | Agentes con comandos viejos (`-File probar.ps1`) en el contexto de sesiones abiertas | Media / bajo | Commit atómico; el error es inmediato ("no existe el archivo"), no silencioso |
| R7 | Hábitos del usuario (doble clic en `ENSAYO.cmd` de la raíz) | Media / bajo | `ENSAYOS.md` actualizado; 8.7 |
| R8 | Copias en `MiYoke_PD_VERSIONES\V1_0..V2_4` tienen la estructura vieja | Cierta / nulo | Cada copia es coherente consigo misma (por eso se descartó resolver la raíz con `git rev-parse`: las copias no tienen `.git`) |
| R9 | Rename no detectado por git (similitud < 50 %) | Baja / nulo | Las ediciones son de pocas líneas; si pasara, `git diff` muestra `D`+`A` y la versión se crea igual |

Riesgo residual fuera de alcance (observado, no se corrige aquí): el acceso directo `INICIAR SIMULADOR V4.lnk` del escritorio apunta a `MiYoke_PD_V2.3\INICIAR SIMULADOR.cmd` (carpeta vieja), no a este repo.

---

## 11. Cómo volver atrás

- **Antes de integrar a `master`**: `git checkout master` y `git branch -D cambio/reorganizar-carpetas` (la rama no tocó `master`). La etiqueta `V2.5` apunta al commit de la rama: borrarla solo a pedido del usuario.
- **Después de integrar**: `git revert --no-edit <commit de la reorganización>` (restaura rutas y textos en un commit nuevo) y crear versión con el script en su ruta vieja: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Revierte reorganizacion de carpetas"`.
- **Inmediato sin git**: la copia `MiYoke_PD_VERSIONES\V2_4` tiene la estructura anterior completa.
- Nada fuera del repo cambia (acceso directo, Prepar3D, Codex), así que volver atrás no requiere tocar configuración del sistema.

---

## 12. Fuera de alcance (anti-scope-creep)

- Mover el firmware a `src/` (cambiaría todos los `#include` del `.ino`, del puente y de las pruebas, y los `AdditionalIncludeDirectories`).
- Renombrar `LEEME md/` (tiene espacio, pero lo citan decenas de documentos y la tesis).
- Unificar `INICIAR SIMULADOR.cmd`/`V3.cmd`, agregar `graphify-ino/` al `.gitignore`, crear un README nuevo en `herramientas/` (el mapa canónico es `INVENTARIO_CODIGO.md`).
- Corregir textos históricos (`DISENO_*`, `VALIDACION_*`, entregas viejas de HANDOFF).

---

## 13. Decisiones

**D1. Nombre de la carpeta: `herramientas/` (minúsculas, sin espacios). Recomendado.**
- Con espacios (`Herramientas de soporte`), cada comando documentado y cada llamada entre scripts requiere comillas. Claude ejecuta desde Git Bash, donde un comando sin comillas se parte en tres argumentos; el error aparecería en el peor momento (lanzamiento automático de Codex, sin usuario presente). `LEEME md/` ya obliga a citar en todo el repo.
- Coherente con el resto del repo: `tesis/`, `tests/`, `ensayos/`, `logs/`, `tmp/` son minúsculas y sin espacios. "de soporte" no agrega información: todo lo que no es producto está ahí.
- Alternativas consideradas: `scripts/` (convención en inglés, pero mezcla idioma) y `tools/` (idem). Se prefiere castellano por consistencia con los documentos.

**D2. `tests/` queda en la raíz. Recomendado.**
- Las pruebas verifican el producto y `comparar_ensayo.exe` lo usan los ensayos: son parte del proyecto, no herramienta. Es la convención habitual en C/C++ (`tests/` junto al código).
- Moverla costaría: 18 `#include "../X.h"` → `"../../X.h"`, `.gitignore` (`tests/bin/`, `tests/obj/`), `launcher_port_checks.ps1` (busca el lanzador en el padre), `probar.ps1`, ~10 documentos LEEME y **6 capítulos de la tesis** (04 a 08 y 10) que citan `tests/...` (requeriría permiso del usuario y edición de Claude). Mucho riesgo para ningún beneficio funcional.

**D3. Helper común para la raíz: no. Recomendado: dos líneas idénticas por script (Bloque RAIZ) con verificación.**
- Un helper (`herramientas/_raiz.ps1`) también hay que encontrarlo relativo al script (`. "$PSScriptRoot\..\_raiz.ps1"`): no elimina la ruta relativa, solo agrega una dependencia más que puede faltar. Además no sirve para `.cmd` ni `.py`.
- `git rev-parse --show-toplevel` se descartó: falla en las copias de `MiYoke_PD_VERSIONES` (no tienen `.git`) y si git no está en el PATH.
- La ruta fija de dos niveles más la verificación de `MiYoke_PD.ino` es determinista, funciona en las copias de versión y, si alguien vuelve a mover un script, falla de inmediato con un mensaje claro en lugar de escribir en un lugar equivocado. Es el patrón de dotnet/arcade.

**D4. PDF duplicado: `git rm`. Recomendado** (requiere el sí del usuario porque es una eliminación): la copia en `tesis/fundamento_efectos/` es idéntica byte a byte y queda en el historial.

**D5. `compilar-tesis.ps1` en `herramientas/flujo/`, no en `tesis/`.** Motivo en C4 (regla "Codex no edita `tesis/`").

**D6. Sin partes para Codex.** Motivo en 7.3 (coherencia del commit atómico). Modelo sugerido: el de código con esfuerzo medium.
