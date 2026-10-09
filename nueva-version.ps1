param([Parameter(Mandatory=$true)][string]$Mensaje)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

# 1. Exigir árbol limpio (se ignora la copia de graphify)
$sucio = git status --porcelain | Where-Object { $_ -notmatch 'graphify-ino/' }
if ($sucio) { throw "Hay cambios sin commit. Hace commit por nombre antes de versionar." }
if (git tag --points-at HEAD) { throw "Este commit ya tiene version: $(git tag --points-at HEAD)" }

# 2. Calcular la version siguiente
$vers = git tag --list 'V*' | Where-Object { $_ -match '^V(\d+)\.(\d+)$' } |
  ForEach-Object { [pscustomobject]@{ Tag=$_; Maj=[int]$Matches[1]; Min=[int]$Matches[2] } } |
  Sort-Object Maj, Min
if ($vers) {
  $ult = $vers[-1]; $maj = $ult.Maj; $min = $ult.Min + 1
  if ($min -gt 9) { $maj++; $min = 0 }
  $prev = $ult.Tag
} else { $maj = 1; $min = 0; $prev = $null }
$tag = "V$maj.$min"; $carpeta = "V${maj}_${min}"

# 3. Etiqueta y copia (solo archivos versionados, sin carpetas ignoradas)
git tag -a $tag -m $Mensaje
$dest = Join-Path (Split-Path $PSScriptRoot -Parent) "MiYoke_PD_VERSIONES\$carpeta"
New-Item -ItemType Directory -Force $dest | Out-Null
$tmp = Join-Path $env:TEMP "$carpeta.tar"
git archive --format=tar -o $tmp HEAD
tar -xf $tmp -C $dest
Remove-Item $tmp

# 4. Archivo de actualizacion
$cambios = if ($prev) { git diff --name-status "$prev" HEAD } else { "Version base." }
$txt = @(
  "VERSION: $tag",
  "FECHA: $(Get-Date -Format 'yyyy-MM-dd HH:mm')",
  "COMMIT: $(git rev-parse --short HEAD)",
  "VERSION ANTERIOR: $(if ($prev) { $prev } else { 'ninguna' })",
  "",
  "CAMBIO: $Mensaje",
  "",
  "ARCHIVOS MODIFICADOS:",
  $cambios
)
Set-Content -Path (Join-Path $dest "ACTUALIZACION_$carpeta.txt") -Value $txt -Encoding UTF8
Write-Host "Version $tag creada en $dest"