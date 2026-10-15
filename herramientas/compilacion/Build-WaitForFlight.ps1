$ErrorActionPreference = 'Stop'
$Raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
if (-not (Test-Path -LiteralPath (Join-Path $Raiz 'MiYoke_PD.ino'))) { throw "No se encontro MiYoke_PD.ino en $Raiz (raiz del repositorio)." }
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $locator)) { throw 'No se encontro vswhere de Visual Studio.' }
$installation = & $locator -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (!$installation) { throw 'No se encontro MSBuild.' }
$builder = Join-Path $installation 'MSBuild\Current\Bin\MSBuild.exe'
& $builder (Join-Path $Raiz 'SimConnectBridge\WaitForFlight.vcxproj') /p:Configuration=Release /p:Platform=Win32 /verbosity:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion del detector de vuelo.' }
Write-Host 'WaitForFlight.exe compilado. No se abrio COM13 ni se ejecuto el puente.'
