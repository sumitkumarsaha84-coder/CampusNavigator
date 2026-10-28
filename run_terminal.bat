@echo off
title Campus Navigator — Terminal CLI Mode
cd /d "%~dp0"
set PATH=C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;%PATH%
CampusNavigatorServer.exe --cli
pause
