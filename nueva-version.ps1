param([Parameter(Mandatory=$true)][string]$Mensaje, [switch]$Forzar)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

# 1. Exigir árbol limpio (se ignora la copia de graphify)
$sucio = git status --porcelain | Where-Object { $_ -notmatch 'graphify-ino/' }
if ($sucio) { throw "Hay cambios sin commit. Hace commit por nombre antes de versionar." }
if (git tag --points-at HEAD) { throw "Este commit ya tiene version: $(git tag --points-at HEAD)" }

# 2. Calcular la version siguiente
$vers = @(git tag --list 'V*' | Where-Object { $_ -match '^V(\d+)\.(\d+)$' } | ForEach-Object { [pscustomobject]@{ Tag=$_; Maj=[int]$Matches[1]; Min=[int]$Matches[2] } } | Sort-Object Maj, Min)
if ($vers.Count -gt 0) {
  $ult = $vers[-1]; $maj = $ult.Maj; $min = $ult.Min + 1
  if ($min -gt 9) { $maj++; $min = 0 }
  $prev = $ult.Tag
} else { $maj = 1; $min = 0; $prev = $null }
if ($prev -and -not $Forzar) {
  $cod = git -c core.quotepath=off diff --name-only $prev HEAD | Where-Object { $_ -match '\.(h|hpp|c|cpp|ino|ps1|cmd|py|vcxproj)$' }
  if (-not $cod) { Write-Host "Sin cambios de codigo desde $prev. No se crea version."; exit 0 }
}
$tag = "V$maj.$min"; $carpeta = "V${maj}_${min}"

# 3. Etiqueta y copia (solo archivos versionados, sin carpetas ignoradas)
git tag -a $tag -m $Mensaje
$dest = Join-Path (Split-Path $PSScriptRoot -Parent) "MiYoke_PD_VERSIONES\$carpeta"
New-Item -ItemType Directory -Force $dest | Out-Null
$tmp = Join-Path $env:TEMP "$carpeta.tar"
git archive --format=tar -o $tmp HEAD
# tar de Windows explicito: el tar de Git Bash interpreta "D:" como host remoto y no copia nada
& (Join-Path $env:SystemRoot 'System32\tar.exe') -xf $tmp -C $dest
$codTar = $LASTEXITCODE
Remove-Item $tmp
$esperados = @(git ls-files).Count
$copiados = @(Get-ChildItem $dest -Recurse -File).Count
if ($codTar -ne 0 -or $copiados -lt $esperados) {
  git tag -d $tag | Out-Null
  Remove-Item -Recurse -Force $dest
  throw "Fallo la copia de la version ($copiados de $esperados archivos). Se elimino la etiqueta $tag; corregir y repetir."
}

# 4. Archivo de actualizacion
$cambios = if ($prev) { git diff --name-status "$prev" HEAD } else { "Version base." }
$txt = @(
  "VERSION: $tag",
  "FECHA: $(Get-Date -Format 'yyyy-MM-dd HH:mm')",
  "COMMIT: $(git rev-parse --short HEAD)",
  "VERSION ANTERIOR: $(if ($prev) { $prev } else { 'ninguna' })",
  "",
  "RAMA: $(git branch --show-current)",
  "CAMBIO: $Mensaje",
  "",
  "ARCHIVOS MODIFICADOS:",
  $cambios
)
Set-Content -Path (Join-Path $dest "ACTUALIZACION_$carpeta.txt") -Value $txt -Encoding UTF8
Write-Host "Version $tag creada en $dest"