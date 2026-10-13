@echo off
setlocal
title Ensayo del Yoke SC8
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0ENSAYO.ps1" %*
pause
