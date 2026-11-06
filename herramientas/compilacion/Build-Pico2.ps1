param(
    [switch]$Encoders,
    [switch]$Potentiometers,
    [switch]$As5600,
    [switch]$PruebaFalla,
    [switch]$Topes,
    [switch]$TopesAs5600,
    [string]$SdkPath = $(if ($env:PICO_SDK_PATH) { $env:PICO_SDK_PATH } else { 'D:/pico/pico-sdk' }),
    [string]$TinyUsbPath = $(if ($env:PICO_TINYUSB_PATH) { $env:PICO_TINYUSB_PATH } else { 'D:/pico/tinyusb' }),
    [string]$ToolchainBin = $(if ($env:PICO_TOOLCHAIN_PATH) { $env:PICO_TOOLCHAIN_PATH } else { 'D:/pico/toolchains/pqt-gcc-5.0.0/bin' }),
    [string]$CMake = 'C:/Program Files/CMake/bin/cmake.exe',
    [string]$Ninja,
    [string]$PicotoolDir = 'D:/pico/picotool/picotool',
    [string]$PioasmDir = 'D:/pico/sdk-tools/pioasm',
    [string]$BuildRoot = $(if ($env:MIYOKE_BUILD_ROOT) { $env:MIYOKE_BUILD_ROOT } else { Join-Path $env:TEMP 'myk' })
)
$ErrorActionPreference = 'Stop'
try {
    if ($As5600 -and ($Encoders -or $Potentiometers -or $PruebaFalla)) { throw '-As5600 no se combina con -Encoders, -Potentiometers ni -PruebaFalla.' }
    if (($Topes -or $TopesAs5600) -and ($Encoders -or $Potentiometers -or $As5600 -or $PruebaFalla -or ($Topes -and $TopesAs5600))) { throw '-Topes y -TopesAs5600 no se combinan con otras variantes.' }
    if ($Encoders -and $Potentiometers) { throw 'Elegir solo -Encoders o -Potentiometers.' }
    if ($PruebaFalla -and $Encoders) { throw '-PruebaFalla usa potenciometros; no combinar con -Encoders.' }
    $Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
    if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'src/main.cpp'))) { throw "No se encontro src/main.cpp en $Raiz" }
    if (-not (Test-Path -LiteralPath $CMake)) { $CMake = (Get-Command cmake -ErrorAction Stop).Source }
    if (-not $Ninja) {
        $ninjaCommand = Get-Command ninja -ErrorAction SilentlyContinue
        $Ninja = if ($ninjaCommand) { $ninjaCommand.Source } else {
            Join-Path $env:LOCALAPPDATA 'Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe/ninja.exe'
        }
    }
    # PICO_TOOLCHAIN_PATH accepts the toolchain root; -ToolchainBin also accepts bin/.
    $toolchainPath = (Resolve-Path -LiteralPath $ToolchainBin).Path
    if ((Split-Path $toolchainPath -Leaf) -eq 'bin') { $toolchainPath = Split-Path $toolchainPath -Parent }
    foreach ($path in @($Ninja, (Join-Path $SdkPath 'pico_sdk_init.cmake'),
                       (Join-Path $TinyUsbPath 'src/tusb.h'),
                       (Join-Path $toolchainPath 'bin/arm-none-eabi-gcc.exe'), $PicotoolDir, $PioasmDir)) {
        if (-not (Test-Path -LiteralPath $path)) { throw "No existe la dependencia: $path" }
    }
    # Una cache configurada con otro SDK, TinyUSB o compilador se descarta: CMake
    # ignoraria los parametros nuevos sin avisar.
    $cachePath = Join-Path $BuildRoot 'CMakeCache.txt'
    if (Test-Path -LiteralPath $cachePath) {
        $cache = Get-Content -LiteralPath $cachePath
        $wanted = @{ PICO_SDK_PATH = $SdkPath; PICO_TINYUSB_PATH = $TinyUsbPath; PICO_TOOLCHAIN_PATH = $toolchainPath }
        foreach ($name in $wanted.Keys) {
            $line = $cache | Where-Object { $_ -match "^$name(:[A-Z]+)?=" } | Select-Object -First 1
            $cached = if ($line) { ($line -split '=', 2)[1] } else { '' }
            if (($cached -replace '\\', '/').TrimEnd('/') -ne ($wanted[$name] -replace '\\', '/').TrimEnd('/')) {
                Write-Host "Reconfigurando: $name cambio ($cached -> $($wanted[$name]))."
                Remove-Item -LiteralPath $cachePath -Force
                Remove-Item -LiteralPath (Join-Path $BuildRoot 'CMakeFiles') -Recurse -Force -ErrorAction SilentlyContinue
                break
            }
        }
    }
    if (-not (Test-Path -LiteralPath $cachePath)) {
        & $CMake -G Ninja -S $Raiz -B $BuildRoot "-DCMAKE_MAKE_PROGRAM=$Ninja" `
            "-DPICO_SDK_PATH=$SdkPath" "-DPICO_TINYUSB_PATH=$TinyUsbPath" `
            "-DPICO_TOOLCHAIN_PATH=$toolchainPath" "-Dpicotool_DIR=$PicotoolDir" `
            "-Dpioasm_DIR=$PioasmDir" '-DCMAKE_BUILD_TYPE=MinSizeRel'
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    $target = if ($Topes) { 'myk_topes' } elseif ($TopesAs5600) { 'myk_topes_as' } elseif ($PruebaFalla) { 'myk_falla' } elseif ($As5600) { 'myk_as' } elseif ($Encoders) { 'myk_enc' } elseif ($Potentiometers) { 'myk_pot' } else { 'myk_cfg' }
    $outputFolder = if ($Topes) { 'build-pico2-topes' } elseif ($TopesAs5600) { 'build-pico2-topes-as5600' } elseif ($PruebaFalla) { 'build-pico2-prueba-falla' } elseif ($As5600) { 'build-pico2-as5600' } elseif ($Encoders) { 'build-pico2-encoders' } elseif ($Potentiometers) { 'build-pico2-potentiometers' } else { 'build-pico2' }
    & $CMake --build $BuildRoot --target $target
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    # Antes de copiar: un UF2 que alcance el sector de calibracion no debe quedar listo para cargar.
    if ((Get-Item -LiteralPath (Join-Path $BuildRoot "$target.bin")).Length -ge 0x3FE000) {
        throw 'El firmware alcanza el sector EEPROM (0x3FE000); no se copio a la carpeta de salida.'
    }
    $outputPath = Join-Path (Join-Path $Raiz 'build') $outputFolder
    New-Item -ItemType Directory -Path $outputPath -Force | Out-Null
    foreach ($ext in @('uf2', 'elf', 'bin', 'map')) {
        $sourceExt = if ($ext -eq 'map') { 'elf.map' } else { $ext }
        Copy-Item -LiteralPath (Join-Path $BuildRoot "$target.$sourceExt") -Destination (Join-Path $outputPath "MiYoke_PD.$ext") -Force
        $oldArtifact = Join-Path $outputPath "MiYoke_PD.ino.$ext"
        if (Test-Path -LiteralPath $oldArtifact) { Remove-Item -LiteralPath $oldArtifact -Force }
    }
    $sizeOutput = @(& (Join-Path $toolchainPath 'bin/arm-none-eabi-size.exe') (Join-Path $outputPath 'MiYoke_PD.elf'))
    if ($LASTEXITCODE -ne 0) { throw 'Fallo arm-none-eabi-size.' }
    $sizes = $sizeOutput[-1].Trim() -split '\s+'
    $flashUsed = [long]$sizes[0] + [long]$sizes[1]
    $ramUsed = [long]$sizes[1] + [long]$sizes[2]
    Write-Host "Flash usada: $flashUsed bytes; RAM usada: $ramUsed bytes"
    Write-Host "Firmware generado en build/$outputFolder. No se cargo en la placa."
    exit 0
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
