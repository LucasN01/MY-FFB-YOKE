param(
    [Parameter(Mandatory=$true)][string]$Entrada,
    [ValidatePattern('^COM\d+$')][string]$Puerto,
    [switch]$Armar,
    [switch]$Confirmado
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name Prepar3D,YokeSimConnectBridge -ErrorAction SilentlyContinue) {
    throw 'Cerra Prepar3D y YokeSimConnectBridge antes del banco: el puerto COM es exclusivo.'
}
if ($Armar -and -not $Confirmado) {
    if ((Read-Host 'Con armado se genera PWM real. Apaga la fuente de motores y escribe APAGADA') -cne 'APAGADA') {
        Write-Host 'Banco cancelado sin armar.'
        exit 1
    }
}
# Codex solo usa -Confirmado con confirmacion humana de fuente apagada, fechada hoy en HANDOFF.
$origen = (Resolve-Path -LiteralPath $Entrada).Path
if (-not $Puerto) {
    $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    try {
        $deteccion = @(& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'Iniciar-Simulador.ps1') -DetectarPlaca 2>&1)
    } finally { $ErrorActionPreference = $prev }
    $coincidencia = [regex]::Match(($deteccion -join "`n"), 'Yoke detectado: (COM\d+)')
    if (-not $coincidencia.Success) { throw 'No se detecto la Pico. Indica el puerto con -Puerto COMn.' }
    $Puerto = $coincidencia.Groups[1].Value
}
$destino = Join-Path $PSScriptRoot ('ensayos\' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '_banco')
New-Item -ItemType Directory -Path $destino | Out-Null
$copia = Join-Path $destino 'entrada.csv'
Copy-Item -LiteralPath $origen -Destination $copia
$registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'
$argumentos = @('--port', $Puerto, '--replay', $copia)
if ($Armar) { $argumentos += '--enable' }
& (Join-Path $registros 'YokeSimConnectBridge.exe') @argumentos | Out-Null
if ($LASTEXITCODE -ne 0) { throw "El puente fallo en el banco (codigo $LASTEXITCODE); revisar bridge.log." }
foreach ($archivo in @('telemetry.csv','pico.csv','ganancias.csv','pico.log','bridge.log')) {
    if (Test-Path -LiteralPath (Join-Path $registros $archivo)) {
        Copy-Item -LiteralPath (Join-Path $registros $archivo) -Destination $destino
    } else { Write-Host "Falta $archivo; se sigue sin el" }
}
if (-not (Test-Path -LiteralPath (Join-Path $destino 'telemetry.csv'))) {
    Write-Host 'Falta telemetry.csv; no se puede comparar el ensayo.'
    exit 1
}
& (Join-Path $PSScriptRoot 'tests\bin\comparar_ensayo.exe') $destino | Out-Null
$codigo = $LASTEXITCODE
Get-Content -LiteralPath (Join-Path $destino 'informe.txt') | ForEach-Object { Write-Host $_ }
exit $codigo
