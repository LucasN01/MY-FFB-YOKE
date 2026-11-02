# Provisorio (parte 2): compila la HAL estatica, sin generar ni cargar firmware.
param(
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
    $Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
    if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'CMakeLists.txt'))) { throw "No se encontro CMakeLists.txt en $Raiz" }
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
    if (-not (Test-Path -LiteralPath (Join-Path $BuildRoot 'CMakeCache.txt'))) {
        & $CMake -G Ninja -S $Raiz -B $BuildRoot "-DCMAKE_MAKE_PROGRAM=$Ninja" `
            "-DPICO_SDK_PATH=$SdkPath" "-DPICO_TINYUSB_PATH=$TinyUsbPath" `
            "-DPICO_TOOLCHAIN_PATH=$toolchainPath" "-Dpicotool_DIR=$PicotoolDir" `
            "-Dpioasm_DIR=$PioasmDir" '-DCMAKE_BUILD_TYPE=MinSizeRel'
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    & $CMake --build $BuildRoot --target myk_hal
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Host 'HAL RP2350 compilada: myk_hal. No se genero ni cargo firmware.'
    exit 0
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
