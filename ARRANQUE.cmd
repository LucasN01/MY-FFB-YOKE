@echo off
setlocal
title Arranque y fallas del Yoke SC8
if not exist "%~dp0tmp\vuelo_corto\telemetry.csv" (
    echo No existe tmp\vuelo_corto\telemetry.csv. Hace primero un ensayo de vuelo corto.
) else (
    "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0ARRANQUE.ps1" -Entrada "%~dp0tmp\vuelo_corto\telemetry.csv" %*
)
pause
