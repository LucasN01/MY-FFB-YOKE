@echo off
setlocal
title Prepar3D + MobiFlight + Yoke SC6
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Iniciar-Simulador.ps1" -Realismo %*
if errorlevel 1 pause
