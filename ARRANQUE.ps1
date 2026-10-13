param(
    [Parameter(Mandatory=$true)][string]$Entrada,
    [ValidatePattern('^COM\d+$')][string]$Puerto,
    [string]$Elf
)
function Select-Elf([string]$boot) {
    $build = if ($boot -match '\betapa=15:PRUEBA\b') { 'build-pico2-prueba-falla' } else { 'build-pico2-potentiometers' }
    Join-Path $PSScriptRoot ($build + '\MiYoke_PD.ino.elf')
}
$ErrorActionPreference = 'Stop'
try {
    if (-not (Test-Path -LiteralPath $Entrada -PathType Leaf)) { throw "No existe la entrada: $Entrada" }
    if (Get-Process -Name Prepar3D,YokeSimConnectBridge -ErrorAction SilentlyContinue) {
        throw 'Cerra Prepar3D y YokeSimConnectBridge antes del arranque: el puerto COM es exclusivo.'
    }
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
    $header = Get-Content -LiteralPath $origen -TotalCount 1
    if (-not $header) { throw 'La entrada no tiene cabecera CSV.' }
    $rows = @(Get-Content -LiteralPath $origen | Select-Object -Skip 1 |
        Where-Object { ($_ | ConvertFrom-Csv -Header ($header -split ',')).sent -eq '1' } |
        Select-Object -First 300)
    if (-not $rows.Count) { throw 'La entrada no tiene filas con sent=1.' }
    $registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'
    $bridge = Join-Path $registros 'YokeSimConnectBridge.exe'
    if (-not (Test-Path -LiteralPath $bridge -PathType Leaf)) { throw "No existe el puente: $bridge" }
    $destino = Join-Path $PSScriptRoot ('ensayos\' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '_arranque')
    New-Item -ItemType Directory -Path $destino | Out-Null
    $copia = Join-Path $destino 'entrada.csv'
    [System.IO.File]::WriteAllLines($copia, [string[]](@($header) + $rows), (New-Object System.Text.UTF8Encoding($false)))
    $startedUtc = [DateTime]::UtcNow
    $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    try {
        & $bridge --port $Puerto --replay $copia 2>&1 | Out-Null
        $bridgeCode = $LASTEXITCODE
    } finally { $ErrorActionPreference = $prev }
    foreach ($archivo in @('bridge.log','pico.log')) {
        $source = Join-Path $registros $archivo
        if ((Test-Path -LiteralPath $source) -and (Get-Item -LiteralPath $source).LastWriteTimeUtc -ge $startedUtc) {
            Copy-Item -LiteralPath $source -Destination $destino
        }
        else { Write-Host "Falta $archivo; se sigue sin el" }
    }
    Write-Host "Puente: codigo $bridgeCode (2=CSV o argumentos, 3=firmware no identificado, 4=enlace perdido). Registros: $destino"
    $logPath = Join-Path $destino 'bridge.log'
    if (-not (Test-Path -LiteralPath $logPath)) { throw 'No se genero bridge.log.' }
    $log = Get-Content -LiteralPath $logPath
    $stack = @($log | Select-String -Pattern 'stack libre minimo')
    if ($stack.Count) { Write-Host $stack[-1].Line }
    $boots = @($log | Select-String -Pattern 'Pico: (BOOT .*)')
    if (-not $boots.Count) {
        Write-Host 'Sin linea BOOT: la Pico no se reinicio desde la ultima conexion (desenchufar y enchufar, o cargar el UF2, y repetir)'
        exit 1
    }
    $boot = $boots[-1].Matches[0].Groups[1].Value
    Write-Host $boot
    $fault = $boot -match '\bfalla=1\b'
    $pcMatch = [regex]::Match($boot, '\bpc=(0x[0-9a-fA-F]+)')
    if ($fault -and $pcMatch.Success) {
        $lrMatch = [regex]::Match($boot, '\blr=(0x[0-9a-fA-F]+)')
        $fallaArgs = @('-Pc', $pcMatch.Groups[1].Value)
        if ($lrMatch.Success) { $fallaArgs += @('-Lr', $lrMatch.Groups[1].Value) }
        $automatico = -not $Elf
        if ($automatico) { $Elf = Select-Elf $boot }
        if (-not $automatico -or (Test-Path -LiteralPath $Elf -PathType Leaf)) {
            if ($automatico) { Write-Host "Elf usado: $Elf (elegido por la linea BOOT; si cargaste otro UF2, repetir FALLA.ps1 con su .elf)" }
            $fallaArgs += @('-Elf', $Elf)
            $output = @(& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'FALLA.ps1') @fallaArgs 2>&1)
            $output | Set-Content -LiteralPath (Join-Path $destino 'falla.txt') -Encoding UTF8
            $output | ForEach-Object { Write-Host $_ }
        } else {
            Write-Host "No existe $Elf"
            $fallaScript = Join-Path $PSScriptRoot 'FALLA.ps1'
            $lrArg = if ($lrMatch.Success) { ' -Lr ' + $lrMatch.Groups[1].Value } else { '' }
            Write-Host ('powershell -ExecutionPolicy Bypass -File "{0}" -Pc {1}{2} -Elf "{3}"' -f
                $fallaScript, $pcMatch.Groups[1].Value, $lrArg, $Elf)
        }
    }
    if ($fault) { exit 2 }
    if ($bridgeCode -ne 0) { exit 1 }
    exit 0
} catch {
    Write-Host "Error: $($_.Exception.Message)"
    exit 1
}
