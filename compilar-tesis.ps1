# compilar-tesis.ps1
# Verifica la tesis con salida minima (ahorro de tokens para agentes).
# Imprime solo "OK ..." o una lista corta de errores con archivo:linea.
# - Si hay latexmk instalado: compila de verdad en tmp\tesis-build.
# - Si no: chequeo estatico (\input existentes, llaves, \begin/\end,
#   \label duplicados, \ref sin \label).
# Uso: powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1 [-Archivo tesis\main.tex] [-MaxErrores 15]

param(
    [string]$Archivo = "tesis\main.tex",
    [int]$MaxErrores = 15
)

$ErrorActionPreference = "Stop"
$raiz = Split-Path -Parent $MyInvocation.MyCommand.Path
$main = Join-Path $raiz $Archivo
if (-not (Test-Path $main)) { Write-Output "ERROR: no existe $Archivo"; exit 2 }
$dirMain = Split-Path -Parent $main

$latexmk = Get-Command latexmk -ErrorAction SilentlyContinue
if ($latexmk) {
    $out = Join-Path $raiz "tmp\tesis-build"
    New-Item -ItemType Directory -Force $out | Out-Null
    Push-Location $dirMain
    try {
        & latexmk -pdf -interaction=nonstopmode -halt-on-error -file-line-error "-outdir=$out" (Split-Path -Leaf $main) *> $null
        $codigo = $LASTEXITCODE
    } finally { Pop-Location }
    $log = Join-Path $out (([IO.Path]::GetFileNameWithoutExtension($main)) + ".log")
    $lineas = @()
    if (Test-Path $log) { $lineas = Get-Content $log -Encoding UTF8 }
    $errores = $lineas | Select-String -Pattern '^(.*:\d+: |! )' | Select-Object -First $MaxErrores
    $refs = ($lineas | Select-String -Pattern 'Reference .* undefined').Count
    if ($codigo -eq 0 -and -not $errores) {
        Write-Output "OK compilacion latexmk ($refs referencias indefinidas). PDF en tmp\tesis-build"
        exit 0
    }
    Write-Output "FALLO compilacion latexmk (codigo $codigo):"
    $errores | ForEach-Object { Write-Output ("  " + $_.Line) }
    exit 1
}

# --- Chequeo estatico (sin LaTeX instalado) ---
$errores = New-Object System.Collections.Generic.List[string]
$labels = @{}
$refs = New-Object System.Collections.Generic.List[object]
$archivos = New-Object System.Collections.Generic.List[string]
$archivos.Add($main)

for ($i = 0; $i -lt $archivos.Count; $i++) {
    $f = $archivos[$i]
    $rel = $f.Substring($raiz.Length + 1)
    $texto = Get-Content $f -Encoding UTF8
    $llaves = 0
    $pila = New-Object System.Collections.Generic.Stack[object]
    for ($n = 0; $n -lt $texto.Count; $n++) {
        # quitar \\, \%, \{, \} y comentarios
        $l = $texto[$n] -replace '\\\\', '  ' -replace '\\[%{}]', '  '
        $p = $l.IndexOf('%')
        if ($p -ge 0) { $l = $l.Substring(0, $p) }
        $num = $n + 1
        foreach ($m in [regex]::Matches($l, '\\(input|include)\{([^}]+)\}')) {
            $nombre = $m.Groups[2].Value
            if (-not $nombre.EndsWith(".tex")) { $nombre += ".tex" }
            $ruta = Join-Path $dirMain $nombre
            if (Test-Path $ruta) { $archivos.Add((Resolve-Path $ruta).Path) }
            else { $errores.Add("${rel}:${num}: \input a archivo inexistente: $nombre") }
        }
        foreach ($m in [regex]::Matches($l, '\\(begin|end)\{([^}]+)\}')) {
            $env = $m.Groups[2].Value
            if ($m.Groups[1].Value -eq "begin") { $pila.Push(@($env, $num)) }
            elseif ($pila.Count -eq 0) { $errores.Add("${rel}:${num}: \end{$env} sin \begin") }
            else {
                $top = $pila.Pop()
                if ($top[0] -ne $env) { $errores.Add("${rel}:${num}: \end{$env} cierra \begin{$($top[0])} de la linea $($top[1])") }
            }
        }
        foreach ($m in [regex]::Matches($l, '\\label\{([^}]+)\}')) {
            $k = $m.Groups[1].Value
            if ($labels.ContainsKey($k)) { $errores.Add("${rel}:${num}: \label{$k} duplicado (antes en $($labels[$k]))") }
            else { $labels[$k] = "${rel}:${num}" }
        }
        foreach ($m in [regex]::Matches($l, '\\(eq|auto|page)?ref\{([^}]+)\}')) {
            $refs.Add(@($m.Groups[2].Value, "${rel}:${num}"))
        }
        foreach ($c in $l.ToCharArray()) {
            if ($c -eq '{') { $llaves++ } elseif ($c -eq '}') { $llaves-- }
        }
        if ($llaves -lt 0) { $errores.Add("${rel}:${num}: llave '}' sin abrir"); $llaves = 0 }
    }
    if ($llaves -ne 0) { $errores.Add("${rel}: $llaves llave(s) '{' sin cerrar al final del archivo") }
    while ($pila.Count -gt 0) { $top = $pila.Pop(); $errores.Add("${rel}:$($top[1]): \begin{$($top[0])} sin \end") }
}
foreach ($r in $refs) {
    if (-not $labels.ContainsKey($r[0])) { $errores.Add("$($r[1]): \ref{$($r[0])} sin \label") }
}

if ($errores.Count -eq 0) {
    Write-Output "OK chequeo estatico ($($archivos.Count) archivos, $($labels.Count) labels). No hay LaTeX instalado: la compilacion real se hace en Overleaf."
    exit 0
}
Write-Output "FALLO chequeo estatico ($($errores.Count) problemas):"
$errores | Select-Object -First $MaxErrores | ForEach-Object { Write-Output "  $_" }
exit 1
