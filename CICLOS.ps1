$ErrorActionPreference = 'Stop'
$destino = Join-Path $PSScriptRoot ('ensayos\' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '_ciclos')
New-Item -ItemType Directory -Path $destino | Out-Null
$tabla = @'
| Paso | Acción | Duración | Para qué |
| --- | --- | --- | --- |
| P0 | UF2 nuevo; lanzador; avión en pista; pausa | — | base |
| P1 | No tocar nada (en pausa) | 10 min | control negativo: 0 `W` |
| P2 | REGISTRO DATOS → Desactivado → OK; luego → Activado → OK | 2 min | control positivo: 2 `W` FLASH; prueba la opción |
| P3 | Menú: recorrer las 4 listas y salir sin guardar, 3 veces, con 1 min entre cada una | 5 min | H2 con puente |
| P4 | Quitar pausa y volar | 5 min | operación |
| P5 | Cerrar el simulador. La Pico queda sola, conectada | 15 min | caso del 12/10 |
| P6 | Sin simulador: menú, cambiar de lista 3 veces, sin guardar | 5 min | H2 + H3 |
| P7 | Sin simulador: `powershell -ExecutionPolicy Bypass -File Iniciar-Simulador.ps1 -DetectarPlaca`, 3 veces, con 1 min entre cada una | 5 min | H1 aislada |
| P8 | Sin simulador: REGISTRO DATOS → Desactivado → OK; esperar 2 min | 3 min | control de `hace` (~120 s) |
| P9 | Abrir el simulador; pausa 2 min; REGISTRO DATOS → Activado → OK; cerrar | 4 min | recibe los eventos de P5-P8 |
'@
Write-Host 'Motores apagados. No desenchufar la Pico: reinicia y borra eventos pendientes.'
Write-Host $tabla
$tabla | Set-Content -LiteralPath (Join-Path $destino 'rutina.txt') -Encoding UTF8
$marcas = Join-Path $destino 'marcas.csv'
'host_time_ms,hora_local,paso' | Set-Content -LiteralPath $marcas -Encoding ASCII
if (-not ('CiclosReloj' -as [type])) {
    Add-Type 'using System.Runtime.InteropServices; public static class CiclosReloj { [DllImport("kernel32.dll")] public static extern ulong GetTickCount64(); }'
}
$paso = 0
do {
    $mensaje = if ($paso -le 9) { "Enter para marcar P$paso, o FIN para terminar despues de cerrar el simulador" } else { 'Escribi FIN para terminar despues de cerrar el simulador' }
    $marca = (Read-Host $mensaje).Trim()
    if ($marca -eq 'FIN') { break }
    if ($marca) { Write-Host 'Usa Enter para marcar el siguiente paso, o FIN.'; continue }
    if ($paso -le 9) {
        "$([CiclosReloj]::GetTickCount64()),$(Get-Date -Format 'HH:mm:ss'),P$paso" | Add-Content -LiteralPath $marcas -Encoding ASCII
        $paso++
    }
} while ($true)
$registros = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8'
foreach ($archivo in @('bridge.log','bridge.anterior.log','pico.log','pico.anterior.log')) {
    $origen = Join-Path $registros $archivo
    if (Test-Path -LiteralPath $origen) { Copy-Item -LiteralPath $origen -Destination $destino }
    else { Write-Host "Falta $archivo; se sigue sin el" }
}
$ciclos = Join-Path $destino 'ciclos.txt'
'' | Set-Content -LiteralPath $ciclos -Encoding UTF8 -NoNewline
foreach ($archivo in @('bridge.anterior.log','bridge.log')) {
    $ruta = Join-Path $destino $archivo
    if (Test-Path -LiteralPath $ruta) {
        Get-Content -LiteralPath $ruta | Where-Object { $_ -match 'ciclo lento|registro CSV' } | Add-Content -LiteralPath $ciclos -Encoding UTF8
    }
}
Write-Host "Registros guardados en $destino"
