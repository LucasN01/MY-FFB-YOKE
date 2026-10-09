@echo off
setlocal
title Prepar3D + MobiFlight + Yoke
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Iniciar-Simulador.ps1" %*
if errorlevel 1 (
  echo.
  echo No se pudo completar el inicio. Revisa el mensaje anterior.
  pause
)
