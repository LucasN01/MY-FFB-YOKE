$ErrorActionPreference = 'Stop'
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $locator)) { throw 'No se encontro vswhere de Visual Studio.' }
$installation = & $locator -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (!$installation) { throw 'No se encontro MSBuild.' }
$builder = Join-Path $installation 'MSBuild\Current\Bin\MSBuild.exe'
& $builder (Join-Path $PSScriptRoot 'SimConnectBridge\WaitForFlight.vcxproj') /p:Configuration=Release /p:Platform=Win32 /verbosity:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion del detector de vuelo.' }
Write-Host 'WaitForFlight.exe compilado. No se abrio COM13 ni se ejecuto el puente.'
