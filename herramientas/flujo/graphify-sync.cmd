@echo off
pushd "%~dp0..\.."
copy /y MiYoke_PD.ino graphify-ino\MiYoke_PD_ino.cpp >nul
graphify update .
popd
