@echo off
title Campus Navigator — Web Dashboard & REST API
cd /d "%~dp0"
set PATH=C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;%PATH%
echo ===============================================================================
echo Starting Campus Navigator C++17 REST API & Web Dashboard...
echo Opening Web Browser at http://localhost:8080 ...
echo ===============================================================================
start http://localhost:8080
CampusNavigatorServer.exe --web --port 8080
pause
