$ErrorActionPreference = 'Stop'
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $locator)) { throw 'No se encontro vswhere. Instalar Visual Studio con herramientas C++.' }
$installation = & $locator -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (!$installation) { throw 'No se encontro MSBuild.' }
$builder = Join-Path $installation 'MSBuild\Current\Bin\MSBuild.exe'
& $builder (Join-Path $PSScriptRoot 'SimConnectBridge\YokeSimConnectBridge.vcxproj') /p:Configuration=Release /p:Platform=Win32 /p:OutDir=bin\Win32\RealismoSC8\ /p:IntDir=obj\Win32\RealismoSC8\ /verbosity:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion del puente.' }
Write-Output 'Puente compilado en SimConnectBridge\bin\Win32\RealismoSC8. No se ejecuto ni se cargo firmware.'
