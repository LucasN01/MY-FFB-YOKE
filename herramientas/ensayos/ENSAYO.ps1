param(
    [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Nombre = 'vuelo1',
    [ValidateSet('completa','corta')][string]$Rutina = 'completa'
)
$ErrorActionPreference = 'Stop'
$Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'MiYoke_PD.ino'))) { throw "No se encontro MiYoke_PD.ino en $Raiz (raiz del repositorio)." }
$destino = Join-Path $Raiz ('ensayos\' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '_' + $Nombre)
New-Item -ItemType Directory -Path $destino | Out-Null
$fases = @(
    @('1','Pausa','15 s','Pausar (P).','armed=0, cmd=0, pwm=0 (B6).'),
    @('2','Menu','30 s','En pausa, recorrer 2-3 pantallas con pulsadores; salir sin guardar.','Menu activo, cmd=0 (B2); ganancias vigentes.'),
    @('3','Parado','20 s','Quitar pausa, quieto con frenos.','gs<=0,5: env=0, suelo=0 (B5).'),
    @('4','Potes parado','30 s','Roll a tope izq/der y pitch adelante/atras despacio, dos veces.','Centrado opuesto a posicion (C); q~0, rigidez de reposo.'),
    @('5','Rodaje','60 s','Soltar frenos; rodar 10 a 30 kt en recta.','env crece, suelo distinto de cero; peak por suelo roll<=40, pitch<=4.'),
    @('6','Despegue','~60 s','Potencia maxima, rotar a ~60 kt, subir a 1000 ft.','on_ground=0: suelo=0 inmediato (B4).'),
    @('7','Crucero','60 s','Nivelado a 100-120 kt, potes quietos.','pressure~(ias/ref)^2; buffet=0, gust=0 sin turbulencia.'),
    @('8','Potes en vuelo','30 s','Repetir F4.','Rigidez de centrado mayor que F4.'),
    @('9','Tren y flaps','60 s','Bajar por etapas dentro de limites y luego subir.','buffet_gear>0 con tren extendido y velocidad; cero retraido.'),
    @('10','Perdida','60 s','Potencia baja, levantar nariz hasta bocina/perdida, recuperar.','buffet_stall crece cerca de stall_alpha.'),
    @('11','Pausa en vuelo','10 s','Pausar (P) y quitar.','cmd=0 en <=150 ms (B6), luego retoma.'),
    @('12','Slew','10 s','Activar Y durante 5 s y desactivar.','Modelos reiniciados y cmd=0 durante slew.'),
    @('13','Turbulencia (opcional)','60 s','Clima con turbulencia moderada.','gust distinto de cero; lanzador ya incluye --gusts.'),
    @('14','Aterrizaje','libre','Aterrizar y rodar hasta parar.','Suelo vuelve al tocar, cero al detenerse.')
)
if ($Rutina -eq 'corta') { $fases = @($fases | Where-Object { $_[0] -in @('1','3','4','5','7','8','11') }) }
$tabla = @(
    'Primera sesion: fuente de potencia de motores apagada. Con motores, solo despues de instalar el pull-down de EN.',
    'Preparacion: INICIAR SIMULADOR V3.cmd, C182RG o habitual, cabecera, motor en marcha, frenos, sin turbulencia, potes centrados.',
    'Seguir el orden y los tiempos. Las fases se detectan automaticamente; no hace falta volver a la consola.',
    '| Fase | Duracion | Que hacer | Que deberia verse |',
    '| --- | --- | --- | --- |'
)
foreach ($fase in $fases) { $tabla += '| F' + $fase[0] + ' ' + ($fase[1..4] -join ' | ') + ' |' }
$tabla | Set-Content -LiteralPath (Join-Path $destino 'rutina.txt') -Encoding UTF8
$marcas = Join-Path $destino 'marcas.csv'
'host_time_ms,fase,titulo' | Set-Content -LiteralPath $marcas -Encoding ASCII
if (-not ('EnsayoReloj' -as [type])) {
    Add-Type 'using System.Runtime.InteropServices; public static class EnsayoReloj { [DllImport("kernel32.dll")] public static extern ulong GetTickCount64(); }'
}
if (-not (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue)) {
    Write-Host 'Primero abrí el simulador con INICIAR SIMULADOR V3.cmd y esperá a estar en la pista'
    Read-Host 'Presiona Enter para continuar' | Out-Null
}
$tabla | ForEach-Object { Write-Host $_ }
Write-Host 'Verifica en el menu: REGISTRO DATOS = Activado'
Read-Host 'En tierra, antes de F1, presiona Enter para marcar inicio' | Out-Null
"$([EnsayoReloj]::GetTickCount64()),inicio,inicio" | Add-Content -LiteralPath $marcas -Encoding ASCII
do {
    $marca = (Read-Host 'Escribi un numero de fase para marcarla, o FIN para terminar (despues de cerrar el simulador)').Trim()
    if ($marca -eq 'FIN') { break }
    if (-not $marca) { continue }
    if ($marca) {
        $fase = $fases | Where-Object { $_[0] -eq $marca } | Select-Object -First 1
        if ($fase) { "$([EnsayoReloj]::GetTickCount64()),F$($fase[0]),$($fase[1])" | Add-Content -LiteralPath $marcas -Encoding ASCII }
        else { Write-Host 'Numero de fase fuera de la rutina seleccionada.' }
    }
} while ($true)
if (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) { Write-Host 'Esperando a que se cierre el puente (cerra el simulador)... hasta 120 s' }
$limite = [EnsayoReloj]::GetTickCount64() + 120000
while ((Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) -and [EnsayoReloj]::GetTickCount64() -lt $limite) { Start-Sleep -Milliseconds 250 }
if (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) { Write-Warning 'El puente sigue abierto tras 120 s; se copian igualmente los registros.' }
$registros = Join-Path $Raiz 'SimConnectBridge\bin\Win32\RealismoSC8'
foreach ($archivo in @('telemetry.csv','pico.csv','ganancias.csv','pico.log','bridge.log')) {
    if (Test-Path -LiteralPath (Join-Path $registros $archivo)) {
        Copy-Item -LiteralPath (Join-Path $registros $archivo) -Destination $destino
    } else { Write-Host "Falta $archivo; se sigue sin el" }
}
if (-not (Test-Path -LiteralPath (Join-Path $destino 'telemetry.csv'))) {
    Write-Host 'Falta telemetry.csv; no se puede comparar el ensayo.'
    exit 1
}
& (Join-Path $Raiz 'tests\bin\comparar_ensayo.exe') $destino | Out-Null
$codigo = $LASTEXITCODE
Get-Content -LiteralPath (Join-Path $destino 'informe.txt') | ForEach-Object { Write-Host $_ }
exit $codigo
