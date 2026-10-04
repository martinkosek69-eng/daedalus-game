@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Invoke-SolarFlight.ps1" -Mode Play -BuildName Build-Aurora26
if errorlevel 1 pause
