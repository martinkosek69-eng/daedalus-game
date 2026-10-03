@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Invoke-Foundation.ps1" -Mode Play
if errorlevel 1 pause
