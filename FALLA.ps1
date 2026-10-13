param(
    [Parameter(Mandatory=$true)][string]$Pc,
    [string]$Lr,
    [Parameter(Mandatory=$true)][string]$Elf
)
$ErrorActionPreference = 'Stop'
try {
    if (-not (Test-Path -LiteralPath $Elf -PathType Leaf)) { throw "No existe el .elf: $Elf" }
    $elfFile = Get-Item -LiteralPath $Elf
    Write-Host ('.elf: {0} (fecha {1})' -f $elfFile.FullName, $elfFile.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))
    Write-Host 'El .elf debe ser de la misma compilacion que el UF2 cargado'
    $pattern = Join-Path $env:LOCALAPPDATA 'Arduino15\packages\rp2040\tools\pqt-gcc\*\bin\arm-none-eabi-addr2line.exe'
    $addr2line = Get-ChildItem -Path $pattern -File -ErrorAction SilentlyContinue |
        Sort-Object { $_.Directory.Parent.LastWriteTime } -Descending | Select-Object -First 1
    if (-not $addr2line) { throw 'No se encontro arm-none-eabi-addr2line.exe en las herramientas de rp2040.' }
    $addresses = @(@{ Name='PC'; Value=$Pc })
    if ($Lr) { $addresses += @{ Name='LR'; Value=$Lr } }
    $failed = $false
    foreach ($item in $addresses) {
        $address = [Convert]::ToUInt64($item.Value.Replace('0x',''),16)
        if ($item.Name -eq 'LR') { $address = $address -band 0xFFFFFFFE }
        $hex = '0x{0:x8}' -f $address
        if (-not (($address -ge 0x10000000 -and $address -lt 0x10400000) -or
                  ($address -ge 0x20000000 -and $address -lt 0x20082000))) {
            Write-Host "$($item.Name) $hex no es direccion de codigo"
            $failed = $true
            continue
        }
        $result = @(& $addr2line.FullName -f -C -e $elfFile.FullName $hex)
        if ($LASTEXITCODE -ne 0 -or $result.Count -lt 2) { throw "addr2line fallo para $($item.Name) $hex" }
        $suffix = if ($item.Name -eq 'LR') { ' (LR = linea siguiente a la llamada)' } else { '' }
        Write-Host "$($item.Name) $hex -> $($result[0]) ($($result[1]))$suffix"
        if ($result[0] -eq '??') { $failed = $true }
    }
    if ($failed) { exit 1 }
    exit 0
} catch {
    Write-Host "Error: $($_.Exception.Message)"
    exit 1
}
