# probar.ps1
# Compila y ejecuta las verificaciones con salida minima (ahorro de tokens para agentes).
# Imprime una linea por paso: "OK ..." o "FALLO ..." con hasta -MaxLineas lineas de error.
# Por defecto: suite tests\simconnect_checks (Release Win32) y tests\launcher_port_checks.ps1.
# Uso: powershell -ExecutionPolicy Bypass -File probar.ps1 [-Pico] [-Encoders] [-Potentiometers] [-Bridge] [-Todo]
#   -Pico            compila firmware Pico 2 con la configuracion del codigo (Build-Pico2.ps1)
#   -Encoders        compila tambien la variante encoders
#   -Potentiometers  compila tambien la variante potenciometros
#   -Bridge          compila el puente SimConnect (Build-Bridge.ps1)
#   -Todo            todo lo anterior
# No carga firmware ni ejecuta el simulador.

param(
    [switch]$Pico,
    [switch]$Encoders,
    [switch]$Potentiometers,
    [switch]$Bridge,
    [switch]$Todo,
    [int]$MaxLineas = 15
)

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
if ($Todo) { $Pico = $true; $Encoders = $true; $Potentiometers = $true; $Bridge = $true }
$fallos = 0

function Resumir([string]$nombre, [int]$codigo, [string[]]$salida, [string]$patronOk, [string]$patronError) {
    if ($codigo -eq 0) {
        $ok = @($salida | Select-String -Pattern $patronOk | ForEach-Object { $_.Line.Trim() } | Where-Object { $_ })
        $detalle = if ($ok.Count) { ' - ' + ($ok -join ' | ') } else { '' }
        Write-Host "OK $nombre$detalle"
        return 0
    }
    Write-Host "FALLO $nombre (codigo $codigo):"
    $err = @($salida | Select-String -Pattern $patronError | Select-Object -First $MaxLineas)
    if (-not $err.Count) { $err = @($salida | Select-Object -Last $MaxLineas) }
    $err | ForEach-Object { Write-Host ("  " + "$_".Trim()) }
    return 1
}

function Ejecutar([scriptblock]$bloque) {
    $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    try { $salida = @(& $bloque 2>&1 | ForEach-Object { "$_" }); $codigo = $LASTEXITCODE }
    catch { $salida = @("$_"); $codigo = 1 }
    finally { $ErrorActionPreference = $prev }
    if ($null -eq $codigo) { $codigo = 0 }
    return @{ Codigo = $codigo; Salida = $salida }
}

# 1. Suite de pruebas C++
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$msbuild = $null
if (Test-Path -LiteralPath $locator) {
    $inst = & $locator -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
    if ($inst) { $msbuild = Join-Path $inst 'MSBuild\Current\Bin\MSBuild.exe' }
}
if (-not $msbuild) {
    Write-Host 'FALLO suite: no se encontro MSBuild (vswhere).'; $fallos++
} else {
    $r = Ejecutar { & $msbuild 'tests\simconnect_checks.vcxproj' /p:Configuration=Release /p:Platform=Win32 /verbosity:quiet /nologo }
    if ($r.Codigo -ne 0) {
        $fallos += Resumir 'compilacion suite' $r.Codigo $r.Salida '^$' '(error|warning) [A-Z]+\d+'
    } else {
        $r = Ejecutar { & 'tests\bin\simconnect_checks.exe' }
        $fallos += Resumir 'suite simconnect_checks' $r.Codigo $r.Salida '^PASS' 'FAIL'
    }
    $r = Ejecutar { & $msbuild 'tests\comparar_ensayo.vcxproj' /p:Configuration=Release /p:Platform=Win32 /verbosity:quiet /nologo }
    $fallos += Resumir 'comparar_ensayo' $r.Codigo $r.Salida '^$' '(error|warning) [A-Z]+\d+'
}

# 2. Pruebas del lanzador (solo funciones, sin puertos)
$r = Ejecutar { & powershell -NoProfile -ExecutionPolicy Bypass -File 'tests\launcher_port_checks.ps1' }
$fallos += Resumir 'launcher_port_checks' $r.Codigo $r.Salida 'PASS|OK' '(?i)error|fail|exception|excepcion'

# 3. Firmware y puente (opcionales)
$patronTam = '(?i)(sketch uses|el sketch usa|global variables|las variables globales)'
$patronErr = '(?i)(error|warning|advertencia)'
if ($Pico) {
    $r = Ejecutar { & powershell -NoProfile -ExecutionPolicy Bypass -File 'Build-Pico2.ps1' }
    $fallos += Resumir 'Pico 2 (configurado)' $r.Codigo $r.Salida $patronTam $patronErr
}
if ($Encoders) {
    $r = Ejecutar { & powershell -NoProfile -ExecutionPolicy Bypass -File 'Build-Pico2.ps1' -Encoders }
    $fallos += Resumir 'Pico 2 (encoders)' $r.Codigo $r.Salida $patronTam $patronErr
}
if ($Potentiometers) {
    $r = Ejecutar { & powershell -NoProfile -ExecutionPolicy Bypass -File 'Build-Pico2.ps1' -Potentiometers }
    $fallos += Resumir 'Pico 2 (potenciometros)' $r.Codigo $r.Salida $patronTam $patronErr
}
if ($Bridge) {
    $r = Ejecutar { & powershell -NoProfile -ExecutionPolicy Bypass -File 'Build-Bridge.ps1' }
    $fallos += Resumir 'puente SimConnect' $r.Codigo $r.Salida '^$' '(error|warning) [A-Z]+\d+'
}

if ($fallos -gt 0) { exit 1 }
exit 0
