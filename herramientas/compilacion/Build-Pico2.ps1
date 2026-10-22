param(
    [switch]$Encoders,
    [switch]$Potentiometers,
    [switch]$As5600,
    [switch]$PruebaFalla,
    [string]$ArduinoCli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
)
$ErrorActionPreference = 'Stop'
$Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'MiYoke_PD.ino'))) { throw "No se encontro MiYoke_PD.ino en $Raiz (raiz del repositorio)." }
if ($As5600 -and ($Encoders -or $Potentiometers -or $PruebaFalla)) { throw '-As5600 no se combina con -Encoders, -Potentiometers ni -PruebaFalla.' }
if ($Encoders -and $Potentiometers) { throw 'Elegir solo -Encoders o -Potentiometers.' }
if ($PruebaFalla -and $Encoders) { throw '-PruebaFalla usa potenciometros; no combinar con -Encoders.' }
if (!(Test-Path -LiteralPath $ArduinoCli)) {
    $ArduinoCli = (Get-Command arduino-cli -ErrorAction Stop).Source
}
$mode = if ($PruebaFalla) { 'prueba-falla' } elseif ($As5600) { 'as5600' } elseif ($Encoders) { 'encoders' } elseif ($Potentiometers) { 'potentiometers' } else { 'configured' }
$outputFolder = if ($PruebaFalla) { 'build-pico2-prueba-falla' } elseif ($As5600) { 'build-pico2-as5600' } elseif ($Encoders) { 'build-pico2-encoders' } elseif ($Potentiometers) { 'build-pico2-potentiometers' } else { 'build-pico2' }
$buildArgs = @('compile', '--fqbn', 'rp2040:rp2040:rpipico2:usbstack=tinyusb,arch=arm',
    '--build-path', (Join-Path $env:TEMP "miyoke-pico2-$mode"),
    '--output-dir', (Join-Path $Raiz $outputFolder))
if ($PruebaFalla) { $buildArgs += @('--build-property', 'build.extra_flags=-DYOKE_POSITION_SOURCE=2 -DYOKE_TEST_HANG=3 -DYOKE_TEST_HANG_MS=20000') }
elseif ($As5600) { $buildArgs += @('--build-property', 'build.extra_flags=-DYOKE_POSITION_SOURCE=3') }
elseif ($Encoders) { $buildArgs += @('--build-property', 'build.extra_flags=-DYOKE_POSITION_SOURCE=0') }
elseif ($Potentiometers) { $buildArgs += @('--build-property', 'build.extra_flags=-DYOKE_POSITION_SOURCE=2') }
$buildArgs += $Raiz
& $ArduinoCli @buildArgs
if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de Pico 2.' }
Write-Output "Firmware generado en $outputFolder. No se cargo en la placa."
