@echo off
title Campus Navigator — Automated ADSA Verification Suite
cd /d "%~dp0"
set PATH=C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin;%PATH%
if not exist "CampusNavigatorTests.exe" (
    echo Building CampusNavigatorTests.exe...
    g++ -std=c++17 -Wall -Wextra -O2 -Iinclude tests/test_main.cpp src/CampusGraph.cpp src/PathFinder.cpp src/CampusData.cpp src/TerminalUI.cpp src/HttpServer.cpp -lws2_32 -o CampusNavigatorTests.exe
)
CampusNavigatorTests.exe
pause
