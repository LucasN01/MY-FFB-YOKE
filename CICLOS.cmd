@echo off
setlocal
title Ciclos lentos del Yoke SC8
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0CICLOS.ps1" %*
pause
